/*
    dmesg.c: Display kernel logs.
    Copyright (C) 2023 streaksu

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <commons.h>

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: dmesg [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("dmesg" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "dmesg: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    size_t length = 100 * 80;
    char *logs = malloc(length);
    int ret, errno;
    SYSCALL2(SYSCALL_DUMPLOGS, logs, length);
    if (ret == -1) {
        return 1;
    }

    for (size_t i = 0; i < length; i += 80) {
       if (logs[i] == '(') {
           printf("%.80s\n", logs + i);
       }
    }
   return 0;
}
