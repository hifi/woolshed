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
#include <string.h>
#include <byteswap.h>

#include "pef.h"
#include "common.h"

static inline void pef_swap32s(int32_t *v)
{
    *v = bswap_32(*v);
}

static inline void pef_swap32(uint32_t *v)
{
    *v = bswap_32(*v);
}

static inline void pef_swap16(uint16_t *v)
{
    *v = bswap_16(*v);
}

// FIXME: this should be using swapping read functions to go through src and making a copy instead of mangling it
void pef_init(PEFImage *pef, void *src, uint32_t size)
{
    uint32_t i;

    /* calculate offsets of each header */
    pef->container = src;
    /*
    pef_swap32(&pef->container->tag1.i);
    pef_swap32(&pef->container->tag2.i);
    */
    pef_swap32(&pef->container->formatVersion);
    pef_swap32(&pef->container->dateTimeStamp);
    pef_swap32(&pef->container->oldDefVersion);
    pef_swap32(&pef->container->oldImpVersion);
    pef_swap32(&pef->container->currentVersion);
    pef_swap16(&pef->container->sectionCount);
    pef_swap16(&pef->container->instSectionCount);
    pef_swap32(&pef->container->reservedA);

    pef->sections = (void *)&pef->container[1];

    for (i = 0; i < pef->container->sectionCount; i++) {
        pef_swap32s(&pef->sections[i].nameOffset);
        pef_swap32(&pef->sections[i].defaultAddress);
        pef_swap32(&pef->sections[i].totalSize);
        pef_swap32(&pef->sections[i].unpackedSize);
        pef_swap32(&pef->sections[i].packedSize);
        pef_swap32(&pef->sections[i].containerOffset);

        if (pef->sections[i].sectionKind == 4)
            pef->loader = (void *)((uint8_t *)pef->container + pef->sections[i].containerOffset);
    }

    if (!pef->loader)
        return;

    pef_swap32s(&pef->loader->mainSection);
    pef_swap32(&pef->loader->mainOffset);
    pef_swap32s(&pef->loader->initSection);
    pef_swap32(&pef->loader->initOffset);
    pef_swap32s(&pef->loader->termSection);
    pef_swap32(&pef->loader->termOffset);
    pef_swap32(&pef->loader->importedLibraryCount);
    pef_swap32(&pef->loader->totalImportedSymbolCount);
    pef_swap32(&pef->loader->relocSectionCount);
    pef_swap32(&pef->loader->relocInstrOffset);
    pef_swap32(&pef->loader->loaderStringsOffset);
    pef_swap32(&pef->loader->exportHashOffset);
    pef_swap32(&pef->loader->exportHashTablePower);
    pef_swap32(&pef->loader->exportedSymbolCount);

    pef->libraries = (void *)&pef->loader[1];

    for (i = 0; i < pef->loader->importedLibraryCount; i++)
    {
        pef_swap32(&pef->libraries[i].nameOffset);
        pef_swap32(&pef->libraries[i].oldImpVersion);
        pef_swap32(&pef->libraries[i].currentVersion);
        pef_swap32(&pef->libraries[i].importedSymbolCount);
        pef_swap32(&pef->libraries[i].firstImportedSymbol);
        pef_swap16(&pef->libraries[i].reservedB);
    }

    pef->symbols = (void *)&pef->libraries[i];

    for (i = 0; i < pef->loader->totalImportedSymbolCount; i++)
    {
        pef_swap32(&pef->symbols[i].i);
    }

    pef->relocSections = (void *)&pef->symbols[i];

    for (i = 0; i < pef->loader->relocSectionCount; i++)
    {
        pef_swap16(&pef->relocSections[i].sectionIndex);
        pef_swap16(&pef->relocSections[i].reservedA);
        pef_swap32(&pef->relocSections[i].relocCount);
        pef_swap32(&pef->relocSections[i].firstRelocOffset);

        uint16_t *relocs = PEFLoaderOffset(pef, pef->loader->relocInstrOffset + pef->relocSections[i].firstRelocOffset);

        for (uint32_t j = 0; j < pef->relocSections[i].relocCount; j++)
        {
            pef_swap16(&relocs[j]);
        }
    }

}

static uint32_t read_arg(uint8_t **src)
{
    uint32_t v = 0;
    uint8_t b;

    do
    {
        b = *(*src)++;
        v = (v << 7) | (b & 0x7F);
    } while (b & 0x80);

    return v;
}

void pef_load_section(PEFImage *pef, uint8_t index, void *ram, uint32_t ram_size)
{
    PEFSectionHeader *section = &pef->sections[index];

    // ignore empty and loader section
    if (section->unpackedSize == 0 || section->sectionKind == 4)
        return;

    printf("Loading section %d as %s at 0x%08X\n", index, sectionKindStr(section->sectionKind), section->defaultAddress);
    
    if (section->sectionKind == 2)
    {
        uint8_t *src = PEFContainerOffset(pef, section->containerOffset);
        uint8_t *end = src + section->packedSize;
        uint8_t *dst = (uint8_t *)ram + section->defaultAddress;

        while (src < end)
        {
            uint8_t opcode = *src >> 5;
            uint32_t count = *src & 0x1F;
            src++;

            if (count == 0)
            {
                count = read_arg(&src);
            }

            switch (opcode)
            {
                case 0: // zero
                    printf(" zero fill %d bytes\n", count);
                    memset(dst, 0, count);
                    dst += count;
                    break;
                case 1: // block copy
                    printf(" block copy %d bytes\n", count);
                    memcpy(dst, src, count);
                    src += count;
                    dst += count;
                    break;
                case 2: // repeatedBlock
                {
                    uint32_t repeatCount = read_arg(&src) + 1;
                    printf(" repeatedBlock %d bytes %d times\n", count, repeatCount);

                    for (uint32_t i = 0; i < repeatCount; i++)
                    {
                        memcpy(dst, src, count);
                        dst += count;
                    }

                    src += count;
                    break;
                }
                case 3: // interleaveRepeatBlockWithBlockCopy
                {
                    uint8_t customSize = read_arg(&src);
                    uint8_t repeatCount = read_arg(&src);
                    uint8_t *commonData = src;
                    src += count;

                    printf(" interleaveRepeatBlockWithBlockCopy(commonSize = %d, customSize = %d, repeatCount = %d)\n", count, customSize, repeatCount);

                    for (int i = 0; i < repeatCount; i++)
                    {
                        // common
                        memcpy(dst, commonData, count);
                        dst += count;

                        // custom
                        memcpy(dst, src, customSize);
                        dst += customSize;
                        src += customSize;

                    }

                    // trailing common
                    memcpy(dst, commonData, count);
                    dst += count;
                    break;
                }
                case 4: // interleaveRepeatBlockWithZero
                {
                    uint8_t customSize = read_arg(&src);
                    uint8_t repeatCount = read_arg(&src);

                    printf(" interleaveRepeatBlockWithZero(commonSize = %d, customSize = %d, repeatCount = %d)\n", count, customSize, repeatCount);

                    for (int i = 0; i < repeatCount; i++)
                    {
                        memset(dst, 0, count);
                        dst += count;

                        memcpy(dst, src, customSize);
                        dst += customSize;
                        src += customSize;
                    }

                    // trailing zeros
                    memset(dst, 0, count);
                    dst += count;

                    break;
                }
                default:
                    printf("ERROR: Unhandled pattern opcode %d\n", opcode);
                    src = end;
                    break;
            }
        }

        return;
    }

    // all other sections go as-is AFAIK
    memcpy((uint8_t *)ram + section->defaultAddress, PEFContainerOffset(pef, section->containerOffset), section->packedSize);
}
