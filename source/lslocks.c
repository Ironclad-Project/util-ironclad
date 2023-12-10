/*
    lslocks.c: List POSIX advisory locks.
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
#include <stdint.h>
#include <unistd.h>
#include <commons.h>
#include <pwd.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/syscall.h>
#include <stdbool.h>
#include <fcntl.h>

#define SC_LIST_FILELOCKS 18
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
    char c;
    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: lslocks [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("lslocks" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "lslocks: %c is not a valid argument\n");
                return 1;
        }
    }

    int ret, errno;
    struct flockinfo *buffer = malloc(20 * sizeof(struct flockinfo));
    SYSCALL3(SYSCALL_SYSCONF, SC_LIST_FILELOCKS, buffer, 20 * sizeof(struct flockinfo));
    if (ret == -1) {
        return 1;
    }

    printf("%4s %4s %20s %20s %10s %10s\n", "PID", "MODE", "START", "LENGTH", "FS", "INO");
    for (int i = 0; i < ret; i++) {
        printf("%4llu %4s %20llu %20llu %10llu %10llu\n",
               buffer[i].pid, buffer[i].mode == MODE_WRITE ? "W" : "R",
               buffer[i].start, buffer[i].length, buffer[i].fs, buffer[i].ino);
    }

    return 0;
}
