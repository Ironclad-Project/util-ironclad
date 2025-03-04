/*
    ipcs.c: Display IPC resources.
    Copyright (C) 2025 streaksu

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
#include <sys/shm.h>
#include <commons.h>
#include <inttypes.h>
#include <sys/syscall.h>

#define MODE_WRITE 0b1

struct flockinfo {
    uint32_t pid;
    uint32_t mode;
    uint64_t start;
    uint64_t length;
    uint64_t fs;
    uint64_t ino;
} __attribute__((packed));

int main(int argc, char *argv[]) {
    int do_shared_segments = 1;
    int do_filelocks = 1;

    char c;
    while ((c = getopt (argc, argv, "hvmla")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ipcs [options]");
                puts("");
                puts("Options:");
                puts("-h  Print this help message");
                puts("-v  Display version information.");
                puts("-m  Display only shared memory segments");
                puts("-l  Display filelocks only");
                puts("-a  Display all resources (default)");
                return 0;
            case 'v':
               puts("ipcs" VERSION_STR);
               return 0;
            case 'm':
               do_shared_segments = 1;
               do_filelocks = 0;
               break;
            case 'l':
               do_shared_segments = 0;
               do_filelocks = 1;
               break;
            case 'a':
               do_shared_segments = 1;
               do_filelocks = 1;
               break;
            default:
                fprintf(stderr, "ipcs: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    if (do_shared_segments) {
        // In Ironclad, shmids go from 1 to X, where X is 20 for current releases.
        // So we can just iterate that.
        struct shmid_ds buf;
        puts("Shared Memory Segments:");
        printf("%10s %5s %10s %10s %10s %6s\n", "key", "shmid", "owner", "perms",
            "bytes", "nattach");
        for (int i = 1; i <= 20; i++) {
            if (!shmctl(i, IPC_STAT, &buf)) {
                printf("%010d %5d %10d %10o %10zu %6ld\n", buf.shm_perm.__ipc_perm_key,
                    i, buf.shm_perm.uid, buf.shm_perm.mode, buf.shm_segsz,
                    buf.shm_nattch);
            }
        }
    }

    if (do_filelocks) {
        if (do_shared_segments) {
            puts("");
        }

        int ret, errno;
        struct flockinfo *buffer = malloc(20 * sizeof(struct flockinfo));
        SYSCALL2(SYSCALL_LISTFLOCKS, buffer, 20);
        if (ret == -1) {
            return 1;
        }

        puts("POSIX filelocks:");
        printf("%4s %4s %20s %20s %10s %10s\n", "PID", "MODE", "START", "LENGTH", "FS", "INO");
        for (int i = 0; i < ret; i++) {
            printf("%4" PRIu32 " %4s %20" PRIu64 " %20" PRIu64 " %10" PRIu64 " %10" PRIu64 "\n",
                   buffer[i].pid, buffer[i].mode == MODE_WRITE ? "W" : "R",
                   buffer[i].start, buffer[i].length, buffer[i].fs, buffer[i].ino);
        }
    }
}
