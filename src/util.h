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

#ifdef __cplusplus
extern "C" {
#endif

void hexdump(void *p, uint32_t len);
uint16_t get_u16(void **pp, void *maxp);
uint32_t get_u32(void **pp, void *maxp);
uint32_t get_str(void **pp, void *maxp, char *str, uint32_t size);

#ifdef __cplusplus
}
#endif
