/*
    pivot_root.c: Pivot roots (duh).
    Copyright (C) 2024 streaksu

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <errno.h>
#include <commons.h>

int main(int argc, char *argv[]) {
    if (argc == 2) {
       if (!strcmp(argv[1], "-h")) {
           puts("Usage: pivot_root [new_root | -h | -v] [old root]");
           puts("");
           puts("Options:");
           puts("-h       Print this help message");
           puts("-v       Display version information");
           return 0;
       } else if (!strcmp(argv[1], "-v")) {
           puts("ipcrm" VERSION_STR);
           return 0;
       } else {
           goto BAD_ARGS_ERR;
       }
    } else if (argc != 3) {
    BAD_ARGS_ERR:
        fputs("pivot_root: exactly 2 arguments are needed\n", stderr);
        return 1;
    }

    int ret;
    SYSCALL4(SYSCALL_PIVOT_ROOT, argv[1], strlen(argv[1]), argv[2], strlen(argv[2]));
    if (ret) {
        perror("pivot_root");
        return 1;
    } else {
        return 0;
    }
}
