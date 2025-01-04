/*
    lspci.c: List all PCI devices.
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
#include <inttypes.h>
#include <time.h>

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: lsclocks [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("lsclocks" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "lsclocks: %c is not a valid argument\n", optopt);
                return 1;
        }
    }

    char buf1[sizeof("2011-10-08T07:07:09Z")];
    char buf2[sizeof("2011-10-08T07:07:09Z")];

    struct timespec mono_res, real_res, mono_time, real_time;
    if (clock_getres(CLOCK_MONOTONIC, &mono_res) ||
        clock_getres(CLOCK_REALTIME,  &real_res)) {
        perror("lsclocks: could not get clock resolution");
        return 1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &mono_time) ||
        clock_gettime(CLOCK_REALTIME,  &real_time)) {
        perror("lsclocks: could not get clock time");
        return 1;
    }

    time_t mono_t = mono_time.tv_sec, real_t = real_time.tv_sec;
    strftime(buf1, sizeof(buf1), "%FT%TZ", gmtime(&mono_t));
    strftime(buf2, sizeof(buf2), "%FT%TZ", gmtime(&real_t));

    printf("%10s %25s %10s %20s\n", "NAME", "TIME", "RESOL", "ISO 8601");
    printf("%10s %15lu.%09lu %8luns %20s\n", "monotonic", mono_time.tv_sec, mono_time.tv_nsec, mono_res.tv_nsec, buf1);
    printf("%10s %15lu.%09lu %8luns %20s\n", "realtime", real_time.tv_sec, real_time.tv_nsec, real_res.tv_nsec, buf2);
}
