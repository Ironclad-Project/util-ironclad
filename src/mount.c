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
#include <commons.h>
#include <errno.h>

#define SC_LIST_MOUNTS 9
struct mountinfo {
    uint32_t type;
    uint32_t flags;
    char     source[20];
    uint32_t source_length;
    char     location[20];
    uint32_t location_length;
    uint64_t blocksize;
    uint64_t fragsize;
    uint64_t sizeinfrags;
    uint64_t freeblocks;
    uint64_t freeblocksu;
    uint64_t inodecount;
    uint64_t freeinodes;
    uint64_t freebinodesu;
    uint64_t maxfile;
};

int string_to_type(const char *type) {
   if (!strncmp(type, "ext", 3) || !strncmp(type, "ext2", 4) ||
       !strncmp(type, "ext3", 4) || !strncmp(type, "ext4", 4)) {
      return MNT_EXT;
   } else if (!strncmp(type, "fat", 3) || !strncmp(type, "fat12", 5) ||
       !strncmp(type, "fat16", 5) || !strncmp(type, "fat32", 5)) {
      return MNT_FAT;
   } else {
      return 0;
   }
}

char *type_to_string(int type) {
   switch (type) {
      case MNT_EXT: return "ext";
      case MNT_FAT: return "fat";
      default: return NULL;
   }
}

int update_mtab(void) {
    long ret, errno;
    struct mountinfo *buffer = malloc(5 * sizeof(struct mountinfo));
    SYSCALL3(SYSCALL_SYSCONF, SC_LIST_MOUNTS, buffer, 5 * sizeof(struct mountinfo));
    if (ret == -1 || ret > 5) {
        return 1;
    }

    FILE *mtab = fopen("/etc/mtab", "w+");
    if (mtab == NULL) {
        perror("mount: could not open mtab");
        return 1;
    }
    for (int i = 0; i < ret; i++) {
        fprintf(mtab, "%.*s ", buffer[i].source_length, buffer[i].source);
        fprintf(mtab, "%.*s ", buffer[i].location_length, buffer[i].location);
        fprintf(mtab, "%s ", type_to_string(buffer[i].type));
        if (buffer[i].flags & MS_RDONLY) {
            fprintf(mtab, "ro");
        } else {
            fprintf(mtab, "rw");
        }
        if (buffer[i].flags & MS_RELATIME) {
            fprintf(mtab, ",relatime");
        }
        fprintf(mtab, " 0 0\n");
    }
    fclose(mtab);
    return 0;
}

int print_kernel_mounts(void) {
    long ret, errno;
    struct mountinfo *buffer = malloc(5 * sizeof(struct mountinfo));
    SYSCALL3(SYSCALL_SYSCONF, SC_LIST_MOUNTS, buffer, 5 * sizeof(struct mountinfo));
    if (ret == -1 || ret > 5) {
        return 1;
    }

    for (int i = 0; i < ret; i++) {
        printf("%.*s ", buffer[i].source_length, buffer[i].source);
        printf("%.*s ", buffer[i].location_length, buffer[i].location);
        printf("type %s ", type_to_string(buffer[i].type));
        if (buffer[i].flags & MS_RDONLY) {
            printf("ro");
        } else {
            printf("rw");
        }
        if (buffer[i].flags & MS_RELATIME) {
            printf(",relatime");
        }
        printf("\n");
    }
    return 0;
}

int update_according_fstab(void) {
    FILE *fstab = fopen("/etc/fstab", "r");
    if (fstab == NULL) {
        perror("mount: could not open fstab");
        return 1;
    }

    char buffer[150];
    while (fgets(buffer, 150, fstab) != NULL) {
        char source[30];
        char target[30];
        char fs[30];

        if (sscanf(buffer, "%s %s %s\n", source, target, fs) != 3) {
            break;
        }
        mount(source, target, string_to_type(fs), 0);
    }

    fclose(fstab);
    return 0;
}

int main(int argc, char *argv[]) {
    char *source = NULL;
    char *target = NULL;
    char *type   = NULL;
    int mount_fstab = 0;
    int flags = 0;

    char c;
    char *tok;
    while ((c = getopt (argc, argv, "hvt:af:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: mount [options] <source> [<target>]");
                puts("");
                puts("Options:");
                puts("-h         Print this help message");
                puts("-v         Print version information");
                puts("-t <type>  FS to mount, or else, try them all!");
                puts("-a         Synchronize mounts according to fstab");
                puts("-f <flags> Flags to mount with");
                puts("");
                puts("Available comma-separated flags:");
                puts("remount   Instead of mounting, remount <source>");
                puts("ro        Mount read-only, if not present, read-write");
                puts("relatime  Use relative timestamps");
                return 0;
            case 'v':
               puts("mount" VERSION_STR);
               return 0;
            case 't':
                type = strdup(optarg);
                if (type == NULL) {
                    return 1;
                }
                break;
            case 'a':
                mount_fstab = 1;
                break;
            case 'f':
                tok = strtok(optarg, ",");
                while (tok != NULL) {
                    if (!strncmp(tok, "remount", 7)) {
                        flags |= MS_REMOUNT;
                    } else if (!strncmp(tok, "ro", 2)) {
                        flags |= MS_RDONLY;
                    } else if (!strncmp(tok, "relatime", 8)) {
                        flags |= MS_RELATIME;
                    } else {
                        fputs("mount: unrecognized flag", stderr);
                        return 1;
                    }
                    tok = strtok(NULL, ",");
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

    int ret;
    if (mount_fstab) {
        ret = update_according_fstab();
    } else if (source == NULL && target == NULL) {
        return print_kernel_mounts();
    } else if (source == NULL) {
        fputs("mount: No source was specified\n", stderr);
        return 1;
    } else if (target == NULL) {
        if (flags & MS_REMOUNT) {
            ret = mount("", source, 0, flags);
        } else {
           fputs("mount: No target was specified\n", stderr);
           return 1;
        }
    } else {
        if (type == NULL) {
            ret = mount(source, target, MNT_EXT, flags);
            if (ret != 0) {
              ret = mount(source, target, MNT_FAT, flags);
            }
        } else {
            ret = mount(source, target, string_to_type(type), 0);
        }
    }

    if (ret) {
        perror("mount: Could not mount");
        return 1;
    }

    update_mtab();
    return 0;
}