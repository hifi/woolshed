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

#include "debug.h"

void debug_printf(const char *fmt, const char *fn, const char *fmt2, ...)
{
#ifdef DEBUG
    va_list ap;

    fprintf(stderr, fmt, fn);

    va_start(ap, fmt2);
    vfprintf(stderr, fmt2, ap);
    va_end(ap);

    fputs("\n", stderr);
#endif
}

void fatal_printf(const char *fmt, const char *fn, const char *fmt2, ...)
{
#ifdef DEBUG
    va_list ap;

    fprintf(stderr, fmt, fn);

    va_start(ap, fmt2);
    vfprintf(stderr, fmt2, ap);
    va_end(ap);

    fputs("\n", stderr);

    exit(1);
#endif
}
