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

int integrity_entrypoint(int argc, char *argv[]) {
    (void)argc;

    bool run_now        = false;
    bool daemonize      = false;
    bool enable_kernel  = false;
    bool disable_kernel = false;

    for (int i = 1; argv[i] != NULL; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            puts("Usage: integrity [options]");
            puts("");
            puts("Options:");
            puts("--now            Run the integrity checks now.");
            puts("--daemonize      Launch a userland daemon for checks.");
            puts("--enable-kernel  Enable the kernel integrity checks.");
            puts("--disable-kernel Disable the kernel integrity checks.");
            puts("-v | --version  Display version information.");
        } else if (!strcmp(argv[i], "--now")) {
            run_now = true;
        } else if (!strcmp(argv[i], "--daemonize")) {
            daemonize = true;
        }else if (!strcmp(argv[i], "--enable-kernel")) {
            enable_kernel = true;
        } else if (!strcmp(argv[i], "--disable-kernel")) {
            disable_kernel = true;
        }
    }

    int ret, errno;
    if (run_now) {
        SYSCALL2(47, 4, 0);
        if (ret != 0) {
            return 1;
        }
    }
    if (enable_kernel) {
        SYSCALL2(47, 2, 60);
        if (ret != 0) {
            return 1;
        }
    }
    if (disable_kernel) {
        SYSCALL2(47, 3, 0);
        if (ret != 0) {
            return 1;
        }
    }

    if (daemonize) {
        pid_t child = fork();
        if (child == 0)  {
            for (;;) {
                sched_yield();
                sched_yield();
                sched_yield();
                sched_yield();
                SYSCALL2(47, 4, 0);
                sched_yield();
                sched_yield();
                sched_yield();
                sched_yield();
            }
        }
    }
    return 0;
}
