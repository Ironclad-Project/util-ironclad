/*
    strace.c: Syscall tracing using ptrace for children processes.
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
#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <poll.h>

struct registers {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

struct thread_info {
    uint16_t tid;
    int is_syscall;
};

struct syscall_info {
    char *name;
    int arg_count;
};

#define MAX_SYSCALL_IDX 94
static const struct syscall_info syscalls[] = {
    [0] = (struct syscall_info){"exit", 1},
    [1] = (struct syscall_info){"arch_prctl", 2},
    [2] = (struct syscall_info){"open", 4},
    [3] = (struct syscall_info){"close", 1},
    [4] = (struct syscall_info){"read", 3},
    [5] = (struct syscall_info){"write", 3},
    [6] = (struct syscall_info){"seek", 3},
    [7] = (struct syscall_info){"mmap", 6},
    [8] = (struct syscall_info){"munmap", 2},
    [9] = (struct syscall_info){"get_pid", 0},
    [10] = (struct syscall_info){"get_ppid", 0},
    [11] = (struct syscall_info){"exec", 6},
    [12] = (struct syscall_info){"clone", 6},
    [13] = (struct syscall_info){"wait", 3},
    [14] = (struct syscall_info){"socket", 2},
    [15] = (struct syscall_info){"set_hostname", 2},
    [16] = (struct syscall_info){"unlink", 3},
    [17] = (struct syscall_info){"fstat", 5},
    [18] = (struct syscall_info){"chdir", 1},
    [19] = (struct syscall_info){"ioctl", 3},
    [20] = (struct syscall_info){"sched_yield", 0},
    [22] = (struct syscall_info){"delete_tcluster", 1},
    [23] = (struct syscall_info){"pipe", 2},
    [24] = (struct syscall_info){"get_uid", 0},
    [25] = (struct syscall_info){"rename", 7},
    [26] = (struct syscall_info){"sysconf", 3},
    [27] = (struct syscall_info){"spawn", 7},
    [28] = (struct syscall_info){"get_tid", 0},
    [29] = (struct syscall_info){"manage_tcluster", 4},
    [30] = (struct syscall_info){"fcntl", 3},
    [31] = (struct syscall_info){"exit_thread", 0},
    [32] = (struct syscall_info){"getrandom", 2},
    [33] = (struct syscall_info){"mprotect", 3},
    [34] = (struct syscall_info){"sync", 0},
    [35] = (struct syscall_info){"set_mac_capabilities", 1},
    [36] = (struct syscall_info){"get_mac_capabilities", 0},
    [37] = (struct syscall_info){"add_mac_permissions", 3},
    [38] = (struct syscall_info){"set_mac_enforcement", 1},
    [39] = (struct syscall_info){"mount", 6},
    [40] = (struct syscall_info){"umount", 3},
    [41] = (struct syscall_info){"readlink", 5},
    [42] = (struct syscall_info){"getdents", 3},
    [43] = (struct syscall_info){"makenode", 5},
    [44] = (struct syscall_info){"truncate", 2},
    [45] = (struct syscall_info){"bind", 3},
    [46] = (struct syscall_info){"symlink", 6},
    [47] = (struct syscall_info){"connect", 3},
    [48] = (struct syscall_info){"openpty", 3},
    [49] = (struct syscall_info){"fsync", 2},
    [50] = (struct syscall_info){"link", 6},
    [51] = (struct syscall_info){"ptrace", 4},
    [52] = (struct syscall_info){"listen", 2},
    [53] = (struct syscall_info){"accept", 4},
    [54] = (struct syscall_info){"getrlimit", 1},
    [55] = (struct syscall_info){"setrlimit", 2},
    [56] = (struct syscall_info){"faccess", 5},
    [57] = (struct syscall_info){"poll", 3},
    [58] = (struct syscall_info){"geteuid", 0},
    [59] = (struct syscall_info){"setuids", 2},
    [60] = (struct syscall_info){"fchmod", 5},
    [61] = (struct syscall_info){"umask", 1},
    [62] = (struct syscall_info){"reboot", 2},
    [63] = (struct syscall_info){"fchown", 6},
    [64] = (struct syscall_info){"pread", 4},
    [65] = (struct syscall_info){"pwrite", 4},
    [66] = (struct syscall_info){"getsockname", 3},
    [67] = (struct syscall_info){"getpeername", 3},
    [68] = (struct syscall_info){"shutdown", 2},
    [69] = (struct syscall_info){"futex", 4},
    [70] = (struct syscall_info){"clock", 3},
    [71] = (struct syscall_info){"clock_nanosleep", 4},
    [72] = (struct syscall_info){"getrusage", 2},
    [73] = (struct syscall_info){"recvfrom", 6},
    [74] = (struct syscall_info){"sendto", 6},
    [75] = (struct syscall_info){"config_netinterface", 3},
    [76] = (struct syscall_info){"utimes", 5},
    [77] = (struct syscall_info){"create_tcluster", 0},
    [78] = (struct syscall_info){"switch_tcluster", 2},
    [79] = (struct syscall_info){"actually_kill", 1},
    [80] = (struct syscall_info){"signalpost", 1},
    [81] = (struct syscall_info){"sendsignal", 2},
    [82] = (struct syscall_info){"getprio", 2},
    [83] = (struct syscall_info){"setprio", 3},
    [84] = (struct syscall_info){"getgid", 0},
    [85] = (struct syscall_info){"getegid", 0},
    [86] = (struct syscall_info){"setgids", 2},
    [87] = (struct syscall_info){"getgroups", 2},
    [88] = (struct syscall_info){"setgroups", 2},
    [89] = (struct syscall_info){"ttyname", 3},
    [90] = (struct syscall_info){"fadvise", 4},
    [91] = (struct syscall_info){"shmat", 3},
    [92] = (struct syscall_info){"shmctl", 3},
    [93] = (struct syscall_info){"shmdt", 1},
    [94] = (struct syscall_info){"shmget", 3}
};

static void print_syscall(FILE *out, struct registers state) {
    if (state.rax > MAX_SYSCALL_IDX) {
        fprintf(out, "(%lu)(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
        state.rax, state.rdi, state.rsi, state.rdx, state.r12,
        state.r8, state.r9);
    } else {
        fprintf(out, "%s(", syscalls[state.rax].name);
        switch (syscalls[state.rax].arg_count) {
            case 0:
                break;
            case 1:
                fprintf(out, "0x%lx", state.rdi);
                break;
            case 2:
                fprintf(out, "0x%lx, 0x%lx", state.rdi, state.rsi);
                break;
            case 3:
                fprintf(out, "0x%lx, 0x%lx, 0x%lx", state.rdi, state.rsi,
                  state.rdx);
                break;
            case 4:
                fprintf(out, "0x%lx, 0x%lx, 0x%lx, 0x%lx", state.rdi, state.rsi,
                  state.rdx, state.r12);
                break;
            case 5:
                fprintf(out, "0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx", state.rdi,
                  state.rsi, state.rdx, state.r12, state.r8);
                break;
            case 6:
                fprintf(out, "0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx",
                  state.rdi, state.rsi, state.rdx, state.r12, state.r8, state.r9);
                break;
            default:
                fprintf(out, "too many args");
                break;
        }
        fprintf(out, ")");
    }
}

static void print_error(FILE *out, struct registers state) {
    if (state.rdx) {
       fprintf(out, " = 0x%lx (%lu)", state.rax, state.rdx);
    } else {
       fprintf(out, " = 0x%lx", state.rax);
    }
}

int main(int argc, char *argv[]) {
    FILE *out = stderr;

    int c;
    while ((c = getopt(argc, argv, "hvo:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: strace [options] [command]");
                puts("");
                puts("Options:");
                puts("-h            Print this help message");
                puts("-v|--version  Print version information");
                puts("-r            Print raw info instead of pretty output");
                puts("-o            Output file, else, stderr");
                puts("");
                puts("Command:");
                puts("Command that will be run for tracing");
                return 0;
            case 'o':
                out = fopen(optarg, "w+");
                if (out == NULL) {
                    perror("strace: Could not open output file");
                    return 1;
                }
                break;
            case 'v':
               puts("strace" VERSION_STR);
               return 0;
            default:
                if (optopt == 'o') {
                    fprintf(stderr, "strace: %c needs an argument\n", optopt);
                    return 1;
                } else {
                    goto END_WHILE;
                }
                break;
        }
    }

END_WHILE:
    int pipes[2];
    if (pipe(pipes)) {
        perror("strace: Could not create pipes");
        return 1;
    }

    pid_t child = fork();
    if (child == 0) {
        if (execvp(argv[optind], argv + optind)) {
            perror("strace: Could not launch program");
            return 1;
        }
    }

    int ret, errno, status;
    uint16_t thread_id;
    struct registers state;

    SYSCALL4(SYSCALL_PTRACE, 1, child, 0, pipes[1]);
    if (ret) {
        fprintf(stderr, "strace: Could not do ptrace: %s\n", strerror(errno));
        return 1;
    }

    // Poll for data to translate and print.
    close(pipes[1]);
    struct pollfd polled = {
        .fd      = pipes[0],
        .events  = POLLIN,
        .revents = 0
    };

    // We keep an array as well for each thread we find in order to know how
    // to continue syscalls without being nonsensical.
    int thread_count = 0;
    struct thread_info *infos = NULL;

    while (true) {
        ret = poll(&polled, 1, -1);
        if (ret == -1) {
           perror("strace: Could not poll");
           return 1;
        }
        if (polled.revents & POLLIN) {
           read(pipes[0], &thread_id, sizeof(uint16_t));
           read(pipes[0], &state, sizeof(state));

         PRINTER:
           int found_idx = 0;
           for (int i = 0; i < thread_count; i++) {
               if (infos[i].tid == thread_id) {
                   if (infos[i].is_syscall == true) {
                       fprintf(out, "%d: ", thread_id);
                       print_syscall(out, state);
                       fprintf(out, "\n");
                       infos[i].is_syscall = state.rax == SYSCALL_EXIT ||
                                    state.rax == SYSCALL_EXIT_THREAD ||
                                    state.rax == SYSCALL_EXEC;
                   } else {
                       fprintf(out, "\t%d: ", thread_id);
                       print_error(out, state);
                       fprintf(out, "\n");
                       infos[i].is_syscall = true;
                   }
                   found_idx = 1;
               }
           }
           if (!found_idx) {
               infos = realloc(infos, (++thread_count) * sizeof(struct thread_info));
               if (infos == NULL) {
                   perror("strace: could not allocate thread information");
                   return 1;
               }
               infos[thread_count - 1].tid = thread_id;
               infos[thread_count - 1].is_syscall = true;
               goto PRINTER;
           }
        }
        if (waitpid(child, &status, WNOHANG) == child) {
            break;
        }
    }

    fprintf(out, "+++ exited with %d +++\n", WEXITSTATUS(status));
    return 0;
}
