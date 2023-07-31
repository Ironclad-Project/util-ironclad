/*
    ps.c: Display process statistics.
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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <entrypoints.h>

#define SC_LIST_PROCS   8
#define PROC_IS_TRACED  0b01
#define PROC_EXITED     0b10

struct procinfo {
    char     id[20];
    uint16_t id_len;
    uint16_t ppid;
    uint16_t pid;
    uint32_t uid;
    uint32_t flags;
} __attribute__((packed));

int ps_entrypoint(int argc, char *argv[]) {
    int print_all_users = 0;
    int print_running   = 0;
    char c;
    while ((c = getopt (argc, argv, "hAr")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ps [options]");
                puts("");
                puts("Options:");
                puts("-h             Print this help message");
                puts("-A             Print all processes regardless of user");
                puts("-r             Print running ones only");
                puts("-v | --version Display version information.");
                return 0;
            case 'A': print_all_users = 1; break;
            case 'r': print_running   = 1; break;
            default:
                fprintf(stderr, "ps: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    struct procinfo *buffer = malloc(50 * sizeof(struct procinfo));

    long ret, errno;
    SYSCALL3(SYSCALL_SYSCONF, SC_LIST_PROCS, buffer, 50 * sizeof(struct procinfo));
    if (ret == -1) {
        return 1;
    } else if (ret > 50) {
        return 1;
    }

    uid_t current_uid = getuid();

    printf("%4s %4s %11s %20s\n", "PPID", "PID", "STAT", "CMD");
    for (int i = 0; i < ret; i++) {
        char flags_message[] = "xxx-xxx";
        if (buffer[i].flags & PROC_IS_TRACED) {
            flags_message[0] = 't';
            flags_message[1] = 'r';
            flags_message[2] = 'a';
        }
        if (buffer[i].flags & PROC_EXITED) {
            flags_message[4] = 'e';
            flags_message[5] = 'x';
            flags_message[6] = 'd';
        }
        if ((print_all_users || buffer[i].uid == current_uid) &&
            (!print_running || (buffer[i].flags & PROC_EXITED) == 0)) {
            printf("%4d %4d %11s %20.*s\n", buffer[i].ppid, buffer[i].pid,
                   flags_message, buffer[i].id_len, buffer[i].id);
        }
    }

   return 0;
}
