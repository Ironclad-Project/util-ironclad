/*
    mount.c: Mount filesystems.
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
#include <entrypoints.h>

int mount_entrypoint(int argc, char *argv[]) {
    char *source = NULL;
    char *target = NULL;
    char *type   = NULL;

    char c;
    while ((c = getopt (argc, argv, "ht:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: mount [options] <source> <target>");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Print version information");
                puts("-t <type>       FS to mount, if not present, a guess will be made");
                puts("-v | --version  Display version information.");
                return 0;
            case 't':
                type = strdup(optarg);
                if (type == NULL) {
                    return 1;
                }
                break;
            default:
                if (optopt == 't') {
                    fputs("mount: Option -t requires an argument\n", stderr);
                    return 1;
                } else {
                    goto END_WHILE;
                }
                break;
        }
    }

END_WHILE:
    for(; optind < argc; optind++){
        if (source == NULL) {
            source = strdup(argv[optind]);
            if (source == NULL) {
                return 1;
            }
        } else if (target == NULL) {
            target = strdup(argv[optind]);
            if (target == NULL) {
                return 1;
            }
        } else {
            fprintf(stderr, "mount: Argument '%s' not used\n", argv[optind]);
        }
    }

    if (source == NULL) {
        fputs("mount: No source was specified\n", stderr);
        return 1;
    }

    if (target == NULL) {
        fputs("mount: No target was specified\n", stderr);
        return 1;
    }

    int ret;
    if (type == NULL) {
        ret = mount(source, target, "ext", 0, NULL);
        if (ret < 0) {
            ret = mount(source, target, "fat32", 0, NULL);
        }
    } else {
        ret = mount(source, target, type, 0, NULL) != -1;
    }

    if (ret < 0) {
        perror("mount: Could not mount");
    }
    free(source);
    free(target);
    free(type);
    return ret;
}
