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

#include <stdint.h>
#include <string.h>

#include "mb.h"
#include "io.h"

static bool mb_read(MacBinary *mb, FILE *fh)
{
    bool ret = false;

    memset(mb, 0, sizeof(*mb));

    if (!io_fread8(&mb->oldVersion, fh))
        goto error;

    if (!io_fread8(&mb->fileNameLength, fh))
        goto error;

    if (!io_fread(mb->fileName, sizeof(mb->fileName), fh))
        goto error;

    if (!io_fread(mb->fileType, sizeof(mb->fileType), fh))
        goto error;

    if (!io_fread(mb->fileCreator, sizeof(mb->fileCreator), fh))
        goto error;

    if (!io_fread8(&mb->finderFlags, fh))
        goto error;

    if (!io_fread8(&mb->zero, fh))
        goto error;

    if (!io_fread16(&mb->verticalPosition, fh))
        goto error;

    if (!io_fread16(&mb->horizontalPosition, fh))
        goto error;

    if (!io_fread16(&mb->windowId, fh))
        goto error;

    if (!io_fread8(&mb->protected, fh))
        goto error;

    if (!io_fread8(&mb->zero2, fh))
        goto error;

    if (!io_fread32(&mb->dataLength, fh))
        goto error;

    if (!io_fread32(&mb->resourceLength, fh))
        goto error;

    if (!io_fread32(&mb->created, fh))
        goto error;

    if (!io_fread32(&mb->modified, fh))
        goto error;

    if (!io_fread16(&mb->getInfoLength, fh))
        goto error;

    if (!io_fskip(21, fh))
        goto error;

    if (!io_fread8(&mb->uploadVersion, fh))
        goto error;

    if (!io_fread8(&mb->neededVersion, fh))
        goto error;

    if (!io_fskip(4, fh))
        goto error;

    ret = true;
error:
    return ret;
}

void mb_dump(MacBinary *mb)
{
    printf("oldVersion          = %d\n", mb->oldVersion);
    printf("fileNameLength      = %d\n", mb->fileNameLength);
    printf("fileName            = \"%s\"\n", mb->fileName);
    printf("fileType            = %c%c%c%c\n", mb->fileType[0], mb->fileType[1], mb->fileType[2], mb->fileType[3]);
    printf("fileCreator         = %c%c%c%c\n", mb->fileCreator[0], mb->fileCreator[1], mb->fileCreator[2], mb->fileCreator[3]);
    printf("finderFlags         = 0x%01X\n", mb->finderFlags);
    printf("zero                = %d\n", mb->zero);
    printf("verticalPosition    = %d\n", mb->verticalPosition);
    printf("horizontalPosition  = %d\n", mb->horizontalPosition);
    printf("windowId            = %d\n", mb->windowId);
    printf("protected           = %d\n", mb->protected);
    printf("zero2               = %d\n", mb->zero2);
    printf("dataLength          = %d\n", mb->dataLength);
    printf("resourceLength      = %d\n", mb->resourceLength);
    printf("created             = 0x%08X\n", mb->created);
    printf("modified            = 0x%08X\n", mb->modified);
    printf("getInfoLength       = %d\n", mb->getInfoLength);
    printf("uploadVersion       = %d\n", mb->uploadVersion);
    printf("neededVersion       = %d\n", mb->neededVersion);
}

bool mb_load(MacBinary *mb, FILE *fh)
{
    bool ret = false;
    long offset = ftell(fh);

    if (!mb_read(mb, fh))
        goto error;

    if (mb->oldVersion != 0 || mb->zero != 0 || mb->zero2 != 0)
        goto error;

    if (mb->uploadVersion < 129 || mb->neededVersion != 129)
        goto error;

    ret = true;

error:
    if (!ret)
        fseek(fh, offset, SEEK_SET);

    return ret;
}
