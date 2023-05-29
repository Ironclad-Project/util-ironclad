/*
    integrity.c: Setup and configure Ironclad's integrity system.
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
#include <stdbool.h>
#include <stdint.h>
#include <cpuid.h>
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

static void print_syscall(FILE *out, struct registers state) {
    switch (state.rax) {
        case SYSCALL_EXIT:
            fprintf(out, "exit(0x%lx)\n", state.rdi);
            break;
        case SYSCALL_ARCH_PRCTL:
            fprintf(out, "arch_prctl(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_OPEN:
            fprintf(out, "open(0x%lx, 0x%lx, 0x%lx, 0x%lx)", state.rdi,
                    state.rsi, state.rdx, state.rcx);
            break;
        case SYSCALL_CLOSE:
            fprintf(out, "close(0x%lx)", state.rdi);
            break;
        case SYSCALL_READ:
            fprintf(out, "read(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_WRITE:
            fprintf(out, "write(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_SEEK:
            fprintf(out, "seek(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_MMAP:
            fprintf(out, "mmap(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.rcx, state.r8,
                    state.r9);
            break;
        case SYSCALL_MUNMAP:
            fprintf(out, "munmap(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_GETPID:
            fprintf(out, "getpid()");
            break;
        case SYSCALL_GETPPID:
            fprintf(out, "getppid()");
            break;
        case SYSCALL_EXEC:
            fprintf(out, "exec(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\n",
                    state.rdi, state.rsi, state.rdx, state.rcx, state.r8,
                    state.r9);
            break;
        case SYSCALL_CLONE:
            fprintf(out, "clone(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.rcx, state.r8);
            break;
        case SYSCALL_WAIT:
            fprintf(out, "wait(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_IOCTL:
            fprintf(out, "ioctl(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_GETDENTS:
            fprintf(out, "getdents(0x%lx, 0x%lx, 0x%lx)", state.rdi,
                    state.rsi, state.rdx);
            break;
        case SYSCALL_STAT:
            fprintf(out, "stat(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_SCHED_YIELD:
            fprintf(out, "sched_yield()");
            break;
        case SYSCALL_PIPE:
            fprintf(out, "pipe(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_UNLINK:
            fprintf(out, "unlink(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                         state.rdx);
            break;
        default:
            fprintf(out, "(%lu)(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rax, state.rdi, state.rsi, state.rdx, state.rcx,
                    state.r8, state.r9);
            break;
    }
}

static void print_error(FILE *out, struct registers state) {
    if (state.rdx) {
       fprintf(out, " = 0x%lx (%lu)\n", state.rax, state.rdx);
    } else {
       fprintf(out, " = 0x%lx\n", state.rax);
    }
}

int strace_entrypoint(int argc, char *argv[]) {
    FILE *out = stderr;

    int c;
    while ((c = getopt(argc, argv, "ho:")) != -1) {
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
    struct registers state;

    SYSCALL4(SYSCALL_PTRACE, 1, child, 0, pipes[1]);
    if (ret) {
        perror("strace: Could not do ptrace");
        return 1;
    }

    // Poll for data to translate and print.
    // We also close our writer end so we get POLLHUP when the pipe is broken
    // (process closed), else we can potentially loop forever!
    close(pipes[1]);
    struct pollfd polled = {
        .fd      = pipes[0],
        .events  = POLLIN,
        .revents = 0
    };
    bool is_syscall = true;
    while (true) {
        ret = poll(&polled, 1, -1);
        if (ret == -1) {
           perror("strace: Could not poll");
           return 1;
        }
        if (polled.revents & POLLIN) {
           if (read(pipes[0], &state, sizeof(state)) == sizeof(state)) {
               if (is_syscall == true) {
                   print_syscall(out, state);
                   is_syscall = state.rax == SYSCALL_EXIT        ||
                                state.rax == SYSCALL_EXIT_THREAD ||
                                state.rax == SYSCALL_EXEC;
               } else {
                   print_error(out, state);
                   is_syscall = true;
               }

           }
        }
        if (waitpid(child, &status, WNOHANG) == child) {
            break;
        }
    }

    fprintf(out, "+++ exited with %d +++\n", WEXITSTATUS(status));
    return 0;
}
