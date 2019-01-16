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

#include <stdlib.h>

#ifdef DEBUG
    #include <stdio.h>
    #include <stdarg.h>

    #define FIXME(...) debug_printf("[FIXME] %s ", __func__, __VA_ARGS__)
    #define WARN(...) debug_printf("[WARN] %s ", __func__, __VA_ARGS__)
    #define ERROR(...) fatal_printf("[ERROR] %s ", __func__, __VA_ARGS__)
    #define INFO(...) debug_printf("[INFO] %s ", __func__, __VA_ARGS__)

    void debug_printf(const char *fmt, const char *fn, const char *fmt2, ...);
    void fatal_printf(const char *fmt, const char *fn, const char *fmt2, ...);
#else
    #define FIXME(...)
    #define WARN(...)
    #define ERROR(...) exit(1)
    #define INFO(...)
#endif
