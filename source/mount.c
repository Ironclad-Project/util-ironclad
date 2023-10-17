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
#include <stdint.h>
#include <sys/syscall.h>
#include <math.h>
#include <sys/mount.h>
#include <entrypoints.h>

#define SC_LIST_MOUNTS 9
struct mountinfo {
    uint32_t type;
    uint32_t flags;
    char     source[20];
    uint32_t source_length;
    char     location[20];
    uint32_t location_length;
};

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

    if (source == NULL && target == NULL) {
        long ret, errno;
        struct mountinfo *buffer = malloc(5 * sizeof(struct mountinfo));
        SYSCALL3(SYSCALL_SYSCONF, SC_LIST_MOUNTS, buffer, 5 * sizeof(struct mountinfo));
        if (ret == -1) {
           return 1;
        } else if (ret > 5) {
            return 1;
        }

        for (int i = 0; i < ret; i++) {
            printf("%.*s on %.*s type ", buffer[i].source_length,
                   buffer[i].source, buffer[i].location_length,
                   buffer[i].location);
            if (buffer[i].type == 1) {
                printf("ext\n");
            } else {
                printf("fat\n");
            }
        }
        return 0;
    } else if (source == NULL) {
        fputs("mount: No source was specified\n", stderr);
        return 1;
    } else if (target == NULL) {
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
        ret = mount(source, target, type, 0, NULL);
    }

    if (ret < 0) {
        perror("mount: Could not mount");
        return 1;
    } else {
        return 0;
    }
}
