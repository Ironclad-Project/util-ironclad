/*
    uptime.c: Display uptime.
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

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons.h>
#include <utmpx.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int print_seconds = 0;

    char c;
    while ((c = getopt (argc, argv, "hsv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: uptime [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-s              Display only the uptime epoch seconds");
                puts("-v              Display version information.");
                return 0;
            case 's':
                print_seconds = 1;
                break;
            case 'v':
               puts("uptime" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "uptime: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    struct timespec tp;

    if (print_seconds) {
        clock_gettime(CLOCK_MONOTONIC, &tp);
        printf("%lu\n", tp.tv_sec);
        return 0;
    } else {
        clock_gettime(CLOCK_REALTIME, &tp);
    }

    char time_str[80];
    time_t epoch = tp.tv_sec;
    struct tm *tm = localtime(&epoch);
    strftime(time_str, 80, "%T up", tm);
    printf(" %s ", time_str);

    clock_gettime(CLOCK_MONOTONIC, &tp);

    uint64_t days = tp.tv_sec / 86400;
    if (days != 0) {
        printf("%ld day", days);
        if (days > 1) {
            printf("s");
        }
        printf(", ");
        tp.tv_sec %= 86400;
    }

    uint64_t hours = tp.tv_sec / 3600;
    if (hours != 0) {
        printf("%ld hour", hours);
         if (hours > 1) {
            printf("s");
        }
        printf(", ");
        tp.tv_sec %= 3600;
    }

    printf("%ld:%.2ld, ", tp.tv_sec / 60, tp.tv_sec % 60);

    struct utmpx *u;
    int count = 0;
    setutxent();
    while ((u = getutxent()) != NULL) {
        if (u->ut_type == USER_PROCESS) {
            count++;
        }
    }
    endutxent();
    if (count != 1) {
        printf("%d users, ", count);
    } else {
        printf("1 user, ");
    }

    double loads[3];
    getloadavg(loads, 3);
    printf("load average: %.4lf, %.4lf, %.4lf\n", loads[0], loads[1], loads[2]);
    return 0;
}
