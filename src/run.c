/*
 *  Copyright (C) 2019 Toni Spets <toni.spets@iki.fi>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#include <signal.h>

#include "pef.h"
#include "cleanup.h"
#include "common.h"
#include "emul_ppc.h"
#include "mb.h"
#include "heap.h"
#include "res.h"

#define STACK_SIZE (32 * 1024)

// global PPC CPU state
static emul_ppc_state cpu;

// loaded PEF image
PEFImage pef;

static int do_break;

void handle_int(int sig)
{
    fprintf(stderr, "\nInterrupted.\n");
    do_break = 1;
}

// FIXME: random pointer list to external symbols
typedef int (*ppc_import_t)(emul_ppc_state *);
static ppc_import_t extImports[512];

int run(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    uint32_t length;
    MacBinary mb;

    if (argc < 2)
    {
        fprintf(stderr, "usage: woolshed <file>\n");
        goto cleanup;
    }

    fh = fopen(argv[1], "rb");

    if (!fh)
    {
        perror("Failed to open file");
        goto cleanup;
    }

    memset(&mb, 0, sizeof(mb));
    if (mb_init(&mb, fh))
    {
        printf("Detected MacBinary file: %d bytes of data and %d bytes of resources. Ignoring resources for now.\n", mb.dataLength, mb.resourceLength);
        mb_seek_data(&mb, fh);
        length = mb.dataLength;
    }
    else
    {
        fseek(fh, 0, SEEK_END);
        length = ftell(fh);
        fseek(fh, 0, SEEK_SET);
    }

    image = malloc(length);

    if (!fread(image, length, 1, fh))
    {
        perror("Failed to read file");
        goto cleanup;
    }

    printf("\nRunning image '%s' (%d bytes)\n\n", argv[1], length);

    pef_init(&pef, image, length);

#if 0
    FAIL_IF(pef->tag1.i != PEF_TAG1 || pef->tag2.i != PEF_TAG2, "Invalid PEF header.");
#endif

    emul_ppc_init(&cpu);

    cpu.ram_size = 64*1024*1024; // FIXME: yeah
    cpu.ram = calloc(1, cpu.ram_size);

    heap_init(cpu.ram_size);
    heap_alloc(4096); // zero zone for invalid addresses
    cpu.r[1] = heap_alloc(STACK_SIZE);

    if (mb.resourceLength)
    {
        uint32_t resourcePtr = heap_alloc(mb.resourceLength);
        mb_seek_resource(&mb, fh);
        printf("Resource starting at %ld\n", ftell(fh));
        fread(PPC_PTR(&cpu, resourcePtr), mb.resourceLength, 1, fh);
        printf("Resource read, %d bytes, file position %ld\n", mb.resourceLength, ftell(fh));
        res_init(&cpu, resourcePtr);
    }

    printf("  RAM = %p (%d bytes)\n", cpu.ram, cpu.ram_size);
    printf("  stack = %d bytes\n", STACK_SIZE);

    printf("\n");

    for (uint32_t i = 0; i < pef.container->sectionCount; i++)
    {
        PEFSectionHeader *section = &pef.sections[i];

        heap_align((uint32_t)pow(2, section->alignment));
        section->defaultAddress = heap_alloc(section->totalSize);

        pef_load_section(&pef, i, cpu.ram, cpu.ram_size);
    }

    printf("\n");

    printf("Loading libraries:\n");

    memset(extImports, 0, sizeof(extImports));
    void* libraries[10]; // FIXME: random
    for (uint32_t i = 0; i < pef.loader->importedLibraryCount; i++)
    {
        PEFImportedLibrary *library = &pef.libraries[i];
        const char *libraryName = PEFLoaderString(pef, library->nameOffset);

        char buf[256];
        sprintf(buf, "./%s.so", libraryName);
        printf(" ... %s\n", buf);
        libraries[i] = dlopen(buf, RTLD_LAZY); // FIXME: never free'd
        if (!libraries[i]) {
            printf("Failed to load!\n");
            return 1;
        }
    }
    printf("\n");

    printf("Executing %d relocation sections...\n", pef.loader->relocSectionCount);

    for (uint32_t i = 0; i < pef.loader->relocSectionCount; i++)
    {
        PEFLoaderRelocationHeader *relocSection = &pef.relocSections[i];

        printf(" %d relocations for section %d...\n", relocSection->relocCount, relocSection->sectionIndex);

        uint32_t relocAddress = pef.sections[relocSection->sectionIndex].defaultAddress;
        uint32_t sectionC = pef.sections[0].defaultAddress;
        uint32_t sectionD = pef.sections[1].defaultAddress;

        uint16_t *relocs = PEFLoaderOffset(&pef, pef.loader->relocInstrOffset + relocSection->firstRelocOffset);

        for (uint32_t i = 0; i < relocSection->relocCount; i++) {
            uint8_t opcode = relocs[i] >> 9;

            if ((opcode & 0x60) == 0)
            {
                uint16_t skipCount = relocs[i] >> 6;
                uint16_t relocCount = relocs[i] & 0x3F;

                printf("   RelocBySectDWithSkip skipCount=%d relocCount=%d\n", skipCount, relocCount);

                relocAddress += skipCount * 4;

                uint32_t *relocBase = PPC_PTR_INT(&cpu, relocAddress);
                for (uint16_t i = 0; i < relocCount; i++)
                {
                    relocBase[i] = PPC_INT(PPC_INT(relocBase[i]) + sectionD);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocSmSetSectD)
            {
                uint16_t index = relocs[i] & 0x1FF;
                sectionD = pef.sections[index].defaultAddress;

                printf("    RelocSmSetSectD section=%d\n", index);
                printf("     sectionD = %08X\n", sectionD);
            }
            else if (opcode == kPEFRelocBySectC)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                uint32_t *relocBase = PPC_PTR_INT(&cpu, relocAddress);

                printf("    RelocBySectC * %d\n", runLength);
                for (uint16_t i = 0; i < runLength; i++)
                {
                    relocBase[i] = PPC_INT(PPC_INT(relocBase[i]) + sectionC);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocBySectD)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                uint32_t *relocBase = PPC_PTR_INT(&cpu, relocAddress);

                printf("    RelocBySectD * %d\n", runLength);
                for (uint16_t i = 0; i < runLength; i++)
                {
                    relocBase[i] = PPC_INT(PPC_INT(relocBase[i]) + sectionD);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocImportRun)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                uint32_t *relocBase = PPC_PTR_INT(&cpu, relocAddress);
                uint32_t importIndex = 0;
                heap_align(4096);
                uint32_t importBase = heap_alloc(runLength * 8); // two words per import

                printf("    RelocImportRun for %d imports\n", runLength);
                for (uint16_t i = 0; i < runLength; i++)
                {
                    PEFSymbolTableEntry *symbol = &pef.symbols[importIndex];
                    const char *symbolName = PEFLoaderString(pef, symbol->s.offset);
                    void *sym = 0;
                    printf("     symbol %s", symbolName);
                    if (symbol->s.name == 2)  {
                        char buf[256];
                        sprintf(buf, "ppc_%s", symbolName);

                        for (uint32_t i = 0; i < pef.loader->importedLibraryCount; i++)
                        {
                            sym = dlsym(libraries[i], buf);
                            if (sym)
                                break;
                        }

                        if (sym)
                            printf(" (found!)\n");
                        else
                            printf(" (using stub, %s not found in any lib)\n", buf);

                        // real symbol pointers, could be stored in application memory as well
                        *(void **)(&extImports[importIndex]) = sym;

                        // this is a trampoline to run_import with importIndex
                        uint32_t tmp[2];
                        tmp[0] = PPC_INT(importBase + 4); // relocation
                        tmp[1] = PPC_INT((6 << 26) | importIndex); // magic opcode
                        memcpy((uint8_t *)cpu.ram + importBase, tmp, sizeof(tmp));

                        relocBase[i] = PPC_INT(importBase);
                        importBase += sizeof(tmp);
                    } else {
                        printf(" (warn: setting to red zone)\n");
                        // FIXME: this is bad but no one uses imported variables, right?
                        relocBase[i] = 0;
                    }
                    relocAddress += 4;
                    importIndex++;
                }
            }
            else if ((opcode & 0x78) == kPEFRelocIncrPosition)
            {
                uint16_t offset = (relocs[i] & 0xFFF) + 1;

                printf("    RelocIncrPosition incr=%d\n", offset);
                printf("     relocAddress += %d\n", offset);

                relocAddress += offset;
            }
            else if (opcode == kPEFRelocTVector8)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                printf("    RelocTVector8 * %d\n", runLength);

                for (uint16_t i = 0; i < runLength; i++)
                {
                    *PPC_PTR_INT(&cpu, relocAddress) = PPC_INT(PPC_INT(*PPC_PTR_INT(&cpu, relocAddress)) + sectionC);
                    relocAddress += 4;

                    *PPC_PTR_INT(&cpu, relocAddress) = PPC_INT(PPC_INT(*PPC_PTR_INT(&cpu, relocAddress)) + sectionD);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocVTable8)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                printf("    RelocVTable8 * %d\n", runLength);

                for (uint16_t i = 0; i < runLength; i++)
                {
                    *PPC_PTR_INT(&cpu, relocAddress) = PPC_INT(PPC_INT(*PPC_PTR_INT(&cpu, relocAddress)) + sectionD);
                    relocAddress += 8;
                }
            }
            else
            {
                printf("Warning: Unhandled relocation instruction 0x%02X\n", opcode);
            }
        }
    }

    // set cpu state
    cpu.pc = PPC_INT(*PPC_PTR_INT(&cpu, pef.sections[pef.loader->mainSection].defaultAddress + pef.loader->mainOffset));
    cpu.r[2] = PPC_INT(*PPC_PTR_INT(&cpu, pef.sections[pef.loader->mainSection].defaultAddress + pef.loader->mainOffset + 4));

    printf("Emulation starting at 0x%08X with TOC at 0x%08X\n", cpu.pc, cpu.r[2]);

    signal(SIGINT, handle_int);

    for (int fault; (fault = emul_ppc_run(&cpu, 0)) && !do_break;)
    {
        // handle some faults
        if (fault == PPC_FAULT_INST)
        {
            uint32_t op = PPC_INT(*PPC_PTR_INT(&cpu, cpu.pc - 4));
            uint32_t primop = op >> 26;

            // import call
            if (primop == 6)
            {
                uint32_t idx = (op & 0x3FFFFFF);
                ppc_import_t import = extImports[idx];

                if (!import)
                {
                    PEFSymbolTableEntry *symbol = &pef.symbols[idx];
                    const char *symbolName = PEFLoaderString(pef, symbol->s.offset);
                    printf("PPC: Import '%s' (%d) missing!\n", symbolName, idx);
                    break;
                }

                if (import(&cpu))
                {
                    printf("PPC: Import function requested exit.\n");
                    break;
                }

                cpu.pc = cpu.lr;
            }
            else
            {
                fprintf(stderr, "PPC: Unhandled instruction at %08X\n", cpu.pc - 4);
                emul_ppc_dump(&cpu);
                break;
            }
        }
        else if (fault == PPC_FAULT_MEM)
        {
            fprintf(stderr, "PPC: Address outside RAM at %08X\n", cpu.pc - 4);
            emul_ppc_dump(&cpu);
            break;
        }
    }

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
