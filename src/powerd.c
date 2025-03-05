/*
    powerd.c: simple power daemon
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <commons.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <limits.h>

#define SHUTDOWN_CMD "shutdown -p now"
#define SLEEP_CMD    "shutdown -r now"

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: powerd [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("powerd" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "powerd: %c is not a valid argument\n", optopt);
                return 1;
        }
    }

    int pwr_fd = open("/dev/pwrbutton", O_RDONLY);
    int sle_fd = open("/dev/sleepbutton", O_RDONLY);
    if (pwr_fd == -1 && sle_fd == -1) {
        return 0;
    }

    int idx = 0;
    struct pollfd polled[2];
    if (pwr_fd != -1) {
        polled[idx].fd = pwr_fd;
        polled[idx].events = POLLIN;
        idx += 1;
    }
    if (sle_fd != -1) {
        polled[idx].fd = sle_fd;
        polled[idx].events = POLLIN;
    }

    if (daemon(0, 0)) {
        perror("Could not daemonize");
        return 1;
    }

    for (;;) {
        poll(&polled[0], idx + 1, INT_MAX);
        for (int i = 0; i <= idx; i++) {
            if (polled [i].revents & POLLIN) {
                if (polled [i].fd == pwr_fd) {
                    system(SHUTDOWN_CMD);
                } else {
                    system(SLEEP_CMD);
                }
            }
        }
    }
}
