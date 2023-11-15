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
                    state.rsi, state.rdx, state.r12);
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
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8,
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
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8,
                    state.r9);
            break;
        case SYSCALL_CLONE:
            fprintf(out, "clone(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8, state.r9);
            break;
        case SYSCALL_WAIT:
            fprintf(out, "wait(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_SOCKET:
            fprintf(out, "socket(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_SETHOSTNAME:
            fprintf(out, "sethostname(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_STAT:
            fprintf(out, "stat(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_CHDIR:
            fprintf(out, "chdir(0x%lx)", state.rdi);
            break;
        case SYSCALL_IOCTL:
            fprintf(out, "ioctl(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_SCHED_YIELD:
            fprintf(out, "sched_yield()");
            break;
        case SYSCALL_DELETE_TCLUSTER:
            fprintf(out, "delete_tcluster(0x%lx)", state.rdi);
            break;
        case SYSCALL_PIPE:
            fprintf(out, "pipe(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_GETUID:
            fprintf(out, "getuid()");
            break;
        case SYSCALL_RENAME:
            fprintf(out, "rename(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8,
                    state.r9, state.r10);
            break;
        case SYSCALL_SYSCONF:
            fprintf(out, "sysconf(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_SPAWN:
            fprintf(out, "spawn(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8,
                    state.r9, state.r10);
            break;
        case SYSCALL_GETTID:
            fprintf(out, "gettid()");
            break;
        case SYSCALL_MANAGE_TCLUSTER:
            fprintf(out, "manage_tcluster(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_FCNTL:
            fprintf(out, "fcntl(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_GETRANDOM:
            fprintf(out, "getrandom(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_MPROTECT:
            fprintf(out, "mprotect(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_SET_MAC_CAPABILITIES:
            fprintf(out, "set_mac_caps(0x%lx)", state.rdi);
            break;
        case SYSCALL_GET_MAC_CAPABILITIES:
            fprintf(out, "get_mac_caps()");
            break;
        case SYSCALL_ADD_MAC_PERMISSIONS:
            fprintf(out, "add_mac_perms(0x%lx, 0x%lx, 0x%lx)", state.rdi, state.rsi,
                    state.rdx);
            break;
        case SYSCALL_SET_MAC_ENFORCEMENT:
            fprintf(out, "set_mac_enforcement(0x%lx)", state.rdi);
            break;
        case SYSCALL_MOUNT:
            fprintf(out, "mount(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8, state.r9);
            break;
        case SYSCALL_UMOUNT:
            fprintf(out, "umount(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_READLINK:
            fprintf(out, "readlink(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_GETDENTS:
            fprintf(out, "getdents(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_SYNC:
            fprintf(out, "sync()");
            break;
        case SYSCALL_MAKENODE:
            fprintf(out, "makenode(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_UNLINK:
            fprintf(out, "unlink(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_TRUNCATE:
            fprintf(out, "truncate(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_BIND:
            fprintf(out, "bind(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_SYMLINK:
            fprintf(out, "symlink(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8, state.r9);
            break;
        case SYSCALL_CONNECT:
            fprintf(out, "connect(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_OPENPTY:
            fprintf(out, "openpty(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_FSYNC:
            fprintf(out, "fsync(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_LINK:
            fprintf(out, "link(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8, state.r9);
            break;
        case SYSCALL_PTRACE:
            fprintf(out, "ptrace(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_LISTEN:
            fprintf(out, "listen(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_ACCEPT:
            fprintf(out, "accept(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_GETRLIMIT:
            fprintf(out, "getrlimit(0x%lx)", state.rdi);
            break;
        case SYSCALL_SETRLIMIT:
            fprintf(out, "setrlimit(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_ACCESS:
            fprintf(out, "access(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_POLL:
            fprintf(out, "poll(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_GETEUID:
            fprintf(out, "geteuid()");
            break;
        case SYSCALL_SETUIDS:
            fprintf(out, "setuids(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_FCHMOD:
            fprintf(out, "chmod(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_UMASK:
            fprintf(out, "umask(0x%lx)", state.rdi);
            break;
        case SYSCALL_REBOOT:
            fprintf(out, "reboot(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_FCHOWN:
            fprintf(out, "chown(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12, state.r8);
            break;
        case SYSCALL_PREAD:
            fprintf(out, "pread(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_PWRITE:
            fprintf(out, "pwrite(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_GETSOCKNAME:
            fprintf(out, "getsockname(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_GETPEERNAME:
            fprintf(out, "getpeername(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_SHUTDOWN:
            fprintf(out, "shutdown(0x%lx, 0x%lx)", state.rdi, state.rsi);
            break;
        case SYSCALL_FUTEX:
            fprintf(out, "futex(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        case SYSCALL_CLOCK:
            fprintf(out, "clock(0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx);
            break;
        case SYSCALL_CLOCK_NANOSLEEP:
            fprintf(out, "clock_nanosleep(0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rdi, state.rsi, state.rdx, state.r12);
            break;
        default:
            fprintf(out, "(%lu)(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)",
                    state.rax, state.rdi, state.rsi, state.rdx, state.r12,
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
           read(pipes[0], &thread_id, sizeof(uint16_t));
           if (read(pipes[0], &state, sizeof(state)) == sizeof(state)) {
               if (is_syscall == true) {
                   fprintf(out, "%d: ", thread_id);
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
