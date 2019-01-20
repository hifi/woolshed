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
#include <stdio.h>

size_t io_fread8(uint8_t *v, FILE *fh);
size_t io_fread16(uint16_t *v, FILE *fh);
size_t io_fread32(uint32_t *v, FILE *fh);
size_t io_fread64(uint64_t *v, FILE *fh);
size_t io_fread(void *v, uint32_t len, FILE *fh);
size_t io_fskip(uint32_t len, FILE *fh);
