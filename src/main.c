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
#include <string.h>
#include "common.h"

int dump(int argc, char **argv);
int export(int argc, char **argv);
int run(int argc, char **argv);

void help(char *progname)
{
    fprintf(stderr, "peftool git~%s (c) 2019 Toni Spets\n", REV);
    fprintf(stderr, "https://github.com/hifi/peftool\n\n");
    fprintf(stderr, "usage: %s <command> [args ...]\n\n", progname);
    fprintf(stderr, "commands:"                                                 "\n"
            "    dump   -- dump information about section of executable"        "\n"
            "    export -- export a section"                                    "\n"
            "    run    -- run a MacOS PEF executable on host"                  "\n"
            "    help   -- this information"                                    "\n"
    );
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        help(argv[0]);
        fprintf(stderr, "\nNo command given: please give valid command name as first argument\n\n");
        return EXIT_FAILURE;
    }
    else if (strcmp(argv[1], "dump")   == 0) return dump   (argc - 1, argv + 1);
    else if (strcmp(argv[1], "export") == 0) return export (argc - 1, argv + 1);
    else if (strcmp(argv[1], "run") == 0)    return run    (argc - 1, argv + 1);
    else if (strcmp(argv[1], "help")   == 0)
    {
        help(argv[0]);
        return EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        help(argv[0]);
        return EXIT_FAILURE;
    }
}
