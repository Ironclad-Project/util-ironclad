/*
    umount.c: Unmount filesystems.
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <sys/mount.h>
#include <commons.h>

int main(int argc, char *argv[]) {
    int flags    = 0;
    char *target = NULL;
    char c;
    while ((c = getopt (argc, argv, "hvf")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: umount <target>");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Print version information");
                puts("-f              Force unmounting, even if busy");
                puts("-v              Display version information.");
                return 0;
            case 'f':
               flags |= 1;
               break;
            case 'v':
               puts("umount" VERSION_STR);
               return 0;
            default:
                goto END_WHILE;
        }
    }

END_WHILE:
    for(; optind < argc; optind++){
        if (target == NULL) {
            target = strdup(argv[optind]);
            if (target == NULL) {
                return 1;
            }
        } else {
            fprintf(stderr, "umount: Argument '%s' not used\n", argv[optind]);
        }
    }

    if (target == NULL) {
        fputs("umount: No target was specified\n", stderr);
        return 1;
    }

    int ret = umount(target, flags);
    if (ret == -1) {
        perror("umount: Could not unmount");
    }
    free(target);
    return ret;
}
