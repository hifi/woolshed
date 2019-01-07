/*
 * Copyright (c) 2019 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
