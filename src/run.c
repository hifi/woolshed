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
#include <byteswap.h>
#include <dlfcn.h>

#include "pef.h"
#include "cleanup.h"
#include "common.h"
#include "emul_ppc.h"

void *ram = NULL;
uint32_t ram_size = 64*1024*1024; // FIXME: yeah
uint32_t import_base = 0x3000000; // FIXME: uh, no

// FIXME: random pointer list to external symbols
void *extImports[512];

// FIXME: uh oh, run_import needed this outside run()
PEFImage pef;

// called by emul_ppc, should be handled from cpu state?
void run_import(uint32_t idx, emul_ppc_state *state)
{
    void (*importFunc)(emul_ppc_state *) = extImports[idx];

    if (!importFunc)
    {
        PEFSymbolTableEntry *symbol = &pef.symbols[idx];
        const char *symbolName = PEFLoaderString(pef, symbol->s.offset);
        emul_ppc_dump();
        printf("Import '%s' (%d) missing!\n", symbolName, idx);
        exit(1);
    }

    importFunc(state);
}

int run(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;

    FAIL_IF(argc < 2, "usage: peftool run <image>\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    fclose(fh);
    fh = NULL; // for cleanup

    printf("\nRunning image '%s' (%d bytes)\n\n", argv[1], length);

    FAIL_IF(length < 512, "File too small.\n");

    pef_init(&pef, image, length);

#if 0
    FAIL_IF(pef->tag1.i != PEF_TAG1 || pef->tag2.i != PEF_TAG2, "Invalid PEF header.");
#endif

    ram = calloc(1, ram_size);
    printf("  RAM = %p (%d bytes)\n", ram, ram_size);

    printf("\n");

    for (uint32_t i = 0; i < pef.container->sectionCount; i++)
    {
        PEFSectionHeader *section = &pef.sections[i];

        section->defaultAddress = (i + 1) * 0x1000000; // FIXME: random

        pef_load_section(&pef, i, ram, ram_size);
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
        uint32_t importIndex = 0;
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

                uint32_t *relocBase = (void *)((uint8_t *)ram + relocAddress);
                for (uint16_t i = 0; i < relocCount; i++)
                {
                    relocBase[i] = bswap_32(bswap_32(relocBase[i]) + sectionD);
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
                uint32_t *relocBase = (void *)((uint8_t *)ram + relocAddress);

                printf("    RelocBySectC * %d\n", runLength);
                for (uint16_t i = 0; i < runLength; i++)
                {
                    relocBase[i] = bswap_32(bswap_32(relocBase[i]) + sectionC);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocBySectD)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                uint32_t *relocBase = (void *)((uint8_t *)ram + relocAddress);

                printf("    RelocBySectD * %d\n", runLength);
                for (uint16_t i = 0; i < runLength; i++)
                {
                    relocBase[i] = bswap_32(bswap_32(relocBase[i]) + sectionD);
                    relocAddress += 4;
                }
            }
            else if (opcode == kPEFRelocImportRun)
            {
                uint16_t runLength = (relocs[i] & 0x1FF) + 1;
                uint32_t *relocBase = (void *)((uint8_t *)ram + relocAddress);

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
                        extImports[importIndex] = sym;

                        // this is a trampoline to run_import with importIndex
                        uint32_t tmp[3];
                        tmp[0] = bswap_32(import_base + 4); // relocation
                        tmp[1] = bswap_32((6 << 26) | importIndex); // magic opcode
                        tmp[2] = bswap_32(0x4e800020); // blr so the cpu jumps back
                        memcpy((uint8_t *)ram + import_base, tmp, sizeof(tmp));

                        relocBase[i] = bswap_32(import_base);
                        import_base += sizeof(tmp);
                    } else {
                        printf(" (using memory)\n");
                        // FIXME: these should come from the loaded library so these are dummies and of the wrong size (all words)
                        // however, it seems most programs never use imported variables so might get away with it for a while
                        relocBase[i] = bswap_32(0x3C00000 + (importIndex * 4));
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

                // FIXME: clean this shit up
                for (uint16_t i = 0; i < runLength; i++) {
                    uint32_t tmp = *(uint32_t *)((uint8_t *)ram + relocAddress);
                    tmp = bswap32(tmp);
                    printf("    @ %08X -> %08X + %08X (sectionC)\n", relocAddress, tmp, sectionC);
                    tmp += sectionC;
                    tmp = bswap32(tmp);
                    *(uint32_t *)((uint8_t *)ram + relocAddress) = tmp;

                    relocAddress += 4;

                    tmp = *(uint32_t *)((uint8_t *)ram + relocAddress);
                    tmp = bswap32(tmp);
                    printf("    @ %08X -> %08X + %08X (sectionD)\n", relocAddress, tmp, sectionD);
                    tmp += sectionD;
                    tmp = bswap32(tmp);
                    *(uint32_t *)((uint8_t *)ram + relocAddress) = tmp;

                    relocAddress += 4;
                }
            }
            else
            {
                printf("Warning: Unhandled relocation instruction 0x%02X\n", opcode);
            }
        }
    }

    uint32_t entryPoint = bswap_32(*(uint32_t *)((uint8_t *)ram + pef.sections[pef.loader->mainSection].defaultAddress + pef.loader->mainOffset));
    // is the TOC offset documented anywhere, it just happens to be here?
    uint32_t toc = bswap_32(*(uint32_t *)((uint8_t *)ram + pef.sections[pef.loader->mainSection].defaultAddress + pef.loader->mainOffset + 4));

    printf("Emulation starting at 0x%08X with TOC at 0x%08X\n", entryPoint, toc);
    init_emul_ppc(entryPoint, toc);

    /*
    uint32_t pc = entryPoint;
    int step = 0;
    */

    for (;;) {
        /*printf("Dumping step %i at %08X:\n", step, pc);
        emul_ppc_dump();
        pc =*/ emul_ppc();
        /*getchar();
        step++;*/
    }

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
