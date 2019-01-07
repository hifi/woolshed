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

#include "pef.h"
#include "cleanup.h"
#include "common.h"

int dump(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;

    FAIL_IF(argc < 2, "usage: peftool dump <image>\n");

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    fclose(fh);
    fh = NULL; // for cleanup

    printf("\nDumping image '%s' (%d bytes)\n\n", argv[1], length);

    FAIL_IF(length < 512, "File too small.\n");

    PEFImage pef;

    pef_init(&pef, image, length);

    printf("PEFContainerHeader\n");
    printf("------------------\n");
    printf("tag1            : %08X (%c%c%c%c)\n", pef.container->tag1.i, pef.container->tag1.s[0], pef.container->tag1.s[1], pef.container->tag1.s[2], pef.container->tag1.s[3]);
    printf("tag2            : %08X (%c%c%c%c)\n", pef.container->tag2.i, pef.container->tag2.s[0], pef.container->tag2.s[1], pef.container->tag2.s[2], pef.container->tag2.s[3]);
    printf("architecture    : %08X (%c%c%c%c)\n", pef.container->architecture.i, pef.container->architecture.s[0], pef.container->architecture.s[1], pef.container->architecture.s[2], pef.container->architecture.s[3]);
    printf("formatVersion   : %d\n", pef.container->formatVersion);
    printf("dateTimeStamp   : %08X\n", pef.container->dateTimeStamp);
    printf("oldDefVersion   : %d\n", pef.container->oldDefVersion);
    printf("oldImpVersion   : %d\n", pef.container->oldImpVersion);
    printf("sectionCount    : %d\n", pef.container->sectionCount);
    printf("instSectionCount: %d\n", pef.container->instSectionCount);
    printf("reservedA       : %08X\n", pef.container->reservedA);
    printf("\n");

    //FAIL_IF(pef.container->tag1.i != PEF_TAG1 || pef.container->tag2.i != PEF_TAG2, "Invalid PEF header.\n");

    printf(" section    start      end   length    daddr    vsize  type     share     align\n");
    printf("-------------------------------------------------------------------------------\n");

    for (int i = 0; i < pef.container->sectionCount; i++)
    {
        const PEFSectionHeader *section = &pef.sections[i];
        char *sectionName = "(undef)";

        if (section->nameOffset != -1)
            sectionName = PEFLoaderString(pef, section->nameOffset);

        printf(
            "%8.8s %8"PRIX32" %8"PRIX32" %8"PRIX32" %8"PRIX32" %8"PRIX32"  %-8s %-9s %5d\n",
            sectionName,
            (section->containerOffset),
            (section->containerOffset) + (section->packedSize),
            (section->unpackedSize),
            (section->defaultAddress),
            (section->defaultAddress) + (section->totalSize),
            sectionKindStr(section->sectionKind),
            shareKindStr(section->shareKind),
            (int)pow(2, section->alignment)
        );
    }

    printf("\n");

    if (pef.loader)
    {
        printf("PEFLoaderInfoHeader\n");
        printf("-----------------------------------\n");
        printf("mainSection              = %08X\n", pef.loader->mainSection);
        printf("mainOffset               = %08X\n", pef.loader->mainOffset);
        printf("initSection              = %08X\n", pef.loader->initSection);
        printf("initOffset               = %08X\n", pef.loader->initOffset);
        printf("termSection              = %08X\n", pef.loader->termSection);
        printf("termOffset               = %08X\n", pef.loader->termOffset);
        printf("importedLibraryCount     = %08X\n", pef.loader->importedLibraryCount);
        printf("totalImportedSymbolCount = %08X\n", pef.loader->totalImportedSymbolCount);
        printf("relocSectionCount        = %08X\n", pef.loader->relocSectionCount);
        printf("relocInstrOffset         = %08X\n", pef.loader->relocInstrOffset);
        printf("loaderStringsOffset      = %08X\n", pef.loader->loaderStringsOffset);
        printf("exportHashOffset         = %08X\n", pef.loader->exportHashOffset);
        printf("exportHashTablePower     = %08X\n", pef.loader->exportHashTablePower);
        printf("exportedSymbolCount      = %08X\n", pef.loader->exportedSymbolCount);
        printf("\n");

        for (uint32_t i = 0; i < pef.loader->importedLibraryCount; i++)
        {
            PEFImportedLibrary *library = &pef.libraries[i];
            const char *libraryName = PEFLoaderString(pef, library->nameOffset);

            printf("Library '%s', ver %08X, %d symbols from %d\n", libraryName, library->currentVersion, library->importedSymbolCount, library->firstImportedSymbol);

            for (uint32_t j = 0; j < library->importedSymbolCount; j++)
            {
                PEFSymbolTableEntry *symbol = &pef.symbols[library->firstImportedSymbol + j];
                const char *symbolName = PEFLoaderString(pef, symbol->s.offset);

                printf("  %02X %02X '%s' %08X (%08X)\n", symbol->s.flags, symbol->s.name, symbolName, symbol->s.offset, symbol->i);
            }
        }
    }

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
