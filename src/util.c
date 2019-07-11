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

#include <assert.h>
#include "util.h"
#include <stdio.h>
#include <ctype.h>

void hexdump(void *p, uint32_t len)
{
    uint8_t *d = (uint8_t *)p;

    uint32_t i = 0;
    while (i < len) {
        int inc = (i + 16 < len) ? 16 : len - i;

        printf("%p ", d + i);

        for (int j = 0; j < 16; j++) {
            if (j % 8 == 0) printf(" ");
            if (j < inc)
                printf("%02X ", d[i + j]);
            else
                printf("   ");
        }

        printf(" |");

        for (int j = 0; j < inc; j++)
            printf("%c", isprint(d[i + j]) ? d[i + j] : '.');

        printf("|\n");

        i += inc;
    }
}

uint16_t get_u16(void **pp, void *maxp)
{
    uint16_t **spp = (uint16_t **)pp;

    assert(!maxp || (*spp)+1 <= (uint16_t *)maxp);

    return *((*spp)++);
}

uint32_t get_u32(void **pp, void *maxp)
{
    uint32_t **spp = (uint32_t **)pp;

    assert(!maxp || (*spp)+1 <= (uint32_t *)maxp);

    return *((*spp)++);
}

uint32_t get_str(void **pp, void *maxp, char *str, uint32_t size)
{
    uint8_t **spp = (uint8_t **)pp;

    assert(maxp == NULL); // FIXME
    uint8_t len = *((*spp)++);
    uint8_t i;

    assert(str && size > (uint32_t)len + 1);

    for (i = 0; i < len; i++)
        str[i] = *((*spp)++);

    str[i] = '\0';

    return len;
}
