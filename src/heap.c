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

#include "heap.h"
#include "debug.h"

// FIXME: keep a list of allocations for freeing and reusing holes
static uint32_t heap_pos;
static uint32_t heap_size;

void heap_init(uint32_t size)
{
    heap_pos = 0;
    heap_size = size;
}

void heap_align(uint32_t align)
{
    if (heap_pos % align > 0)
        heap_pos += align - (heap_pos % align);

    if (heap_pos > heap_size)
        ERROR("Heap full.");
}

uint32_t heap_alloc(uint32_t size)
{
    uint32_t ret = heap_pos;

    heap_pos += size;

    if (heap_pos > heap_size)
        ERROR("Heap full.");

    return ret;
}

void heap_free(uint32_t addr)
{
    FIXME("not implemented");
}
