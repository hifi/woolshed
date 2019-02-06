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

#include <stdbool.h>
#include <stdio.h>

#define MACBINARY_SIZE 128

typedef struct {
    uint8_t oldVersion;
    uint8_t fileNameLength;
    char fileName[63];
    char fileType[4];
    char fileCreator[4];
    uint8_t finderFlags;
    uint8_t zero;
    uint16_t verticalPosition;
    uint16_t horizontalPosition;
    uint16_t windowId;
    uint8_t protected;
    uint8_t zero2;
    uint32_t dataLength;
    uint32_t resourceLength;
    uint32_t created;
    uint32_t modified;
    uint16_t getInfoLength;
    uint8_t uploadVersion;
    uint8_t neededVersion;
} MacBinary;

void mb_dump(MacBinary *mb);
bool mb_init(MacBinary *mb, FILE *fh);
void mb_seek_data(MacBinary *mb, FILE *fh);
void mb_seek_resource(MacBinary *mb, FILE *fh);
