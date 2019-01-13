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

int export(int argc, char **argv)
{
    // decleration before more meaningful initialization for cleanup
    int     ret   = EXIT_SUCCESS;
    FILE   *fh    = NULL;
    int8_t *image = NULL;
    int32_t sect_num = 0;

    FAIL_IF(argc < 3, "usage: peftool export <image> <section number>\n");

    sect_num = atoi(argv[2]);

    uint32_t length;
    FAIL_IF_SILENT(open_and_read(&fh, &image, &length, argv[1], "rb"));

    fclose(fh);
    fh = NULL; // for cleanup

    fprintf(stderr, "Exporting section %d from image '%s' (%d bytes)\n", sect_num, argv[1], length);

    FAIL_IF(length < 512, "File too small.\n");

    PEFContainerHeader *pef = (void *)image;

    //FAIL_IF(pef->tag1.i != PEF_TAG1 || pef->tag2.i != PEF_TAG2, "Invalid PEF header.");

    FAIL_IF(sect_num < 0 || sect_num > bswap16(pef->sectionCount), "Invalid section index.");

    const PEFSectionHeader *section = (void *)(image + sizeof(PEFContainerHeader) + (sizeof(PEFSectionHeader) * sect_num));

    fwrite(image + bswap32(section->containerOffset), bswap32(section->packedSize), 1, stdout);

cleanup:
    if (image) free(image);
    if (fh)    fclose(fh);
    return ret;
}
