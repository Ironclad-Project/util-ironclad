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
#include <commons.h>
#include <inttypes.h>
#include <string.h>

#define SC_LIST_PROCS   8
#define SC_LIST_THREADS 12
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

struct threadinfo {
    uint16_t tid;
    int16_t  niceness;
    uint16_t pid;
} __attribute__((packed));

static void print_process_list_header(void) {
    printf("%4s %4s %11s %20s\n", "PPID", "PID", "STAT", "CMD");
}

static void print_process_list_process(struct procinfo *proc) {
    char flags_message[] = "xxx-xxx";
    if (proc->flags & PROC_IS_TRACED) {
        flags_message[0] = 't';
        flags_message[1] = 'r';
        flags_message[2] = 'a';
    }
    if (proc->flags & PROC_EXITED) {
        flags_message[4] = 'e';
        flags_message[5] = 'x';
        flags_message[6] = 'd';
    }
    printf("%4d %4d %11s %20.*s\n", proc->ppid, proc->pid, flags_message,
        proc->id_len, proc->id);
}

int main(int argc, char *argv[]) {
    int print_all_users = 0;
    int print_threads   = 0;
    int print_running   = 0;
    int print_only_this = 0;
    int print_only_this_name = 0;
    int print_only_this_parent = 0;

    char c;
    while ((c = getopt (argc, argv, "hvATCrp:o:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ps [options]");
                puts("");
                puts("Options:");
                puts("-h          Print this help message");
                puts("-v          Display version information.");
                puts("-A          Print all processes regardless of user and filters");
                puts("-T          Print threads instead of processes");
                puts("-r          Print running ones only");
                puts("-p <pid>    Only print information of the passed PID");
                puts("-o <format> Format of process information output");
                return 0;
            case 'v':
               puts("ps" VERSION_STR);
               return 0;
            case 'A': print_all_users = 1; break;
            case 'T': print_threads   = 1; break;
            case 'r': print_running   = 1; break;
            case 'p':
                if (sscanf(optarg, "%d", &print_only_this) != 1) {
                    fprintf(stderr, "ps: '%s' is not a valid PID", optarg);
                }
                break;
            case 'o':
                if (!strncmp(optarg, "comm", 4)) {
                    print_only_this_name = 1;
                } else if (!strncmp(optarg, "ppid", 4)) {
                    print_only_this_parent = 1;
                }
                break;
            default:
                fprintf(stderr, "ps: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    if (print_threads) {
        struct threadinfo *buffer = malloc(50 * sizeof(struct threadinfo));

        long ret, errno;
        SYSCALL3(SYSCALL_SYSCONF, SC_LIST_THREADS, buffer, 50 * sizeof(struct procinfo));
        if (ret == -1 || ret > 50) {
            return 1;
        }

        const long count = ret;
        printf("%4s %4s %4s %20s\n", "TID", "NICE", "PID", "ID");
        for (int i = 0; i < count; i++) {
            char id_buf[64];
            SYSCALL3(SYSCALL_GETTIDID, buffer[i].tid, id_buf, 64);
            if (ret != 0) {
                strcpy(id_buf, " ");
            }

            printf("%4d %4" PRId16 " ", buffer[i].tid, buffer[i].niceness);
            printf("%4d %20.*s\n", buffer[i].pid, (int)strlen(id_buf), id_buf);
        }
    } else {
        struct procinfo *buffer = malloc(50 * sizeof(struct procinfo));

        long ret, errno;
        SYSCALL3(SYSCALL_SYSCONF, SC_LIST_PROCS, buffer, 50 * sizeof(struct procinfo));
        if (ret == -1 || ret > 50) {
            return 1;
        }

        if (print_only_this) {
            for (int i = 0; i < ret; i++) {
                if (buffer[i].pid == print_only_this) {
                    if (print_only_this_name) {
                        printf("%.*s\n", buffer[i].id_len, buffer[i].id);
                    } else if (print_only_this_parent) {
                        printf("%d\n", buffer[i].ppid);
                    } else {
                        print_process_list_header();
                        print_process_list_process(&buffer[i]);
                    }
                    break;
                }
            }
        } else {
            uid_t current_uid = getuid();

            print_process_list_header();
            for (int i = 0; i < ret; i++) {
                if ((print_all_users || buffer[i].uid == current_uid) &&
                    (!print_running || (buffer[i].flags & PROC_EXITED) == 0)) {
                    print_process_list_process(&buffer[i]);
                }
            }
        }
    }

    return 0;
}