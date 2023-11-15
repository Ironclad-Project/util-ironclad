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
#include <stdio.h>
#include <unistd.h>
#include <commons.h>

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
        printf("%llu\n", tp.tv_sec);
        return 0;
    } else {
        clock_gettime(CLOCK_REALTIME, &tp);
    }

    char time_str[80];
    time_t epoch = tp.tv_sec;
    struct tm *tm = localtime(&epoch);
    size_t time_str_len = strftime(time_str, 80, "%T up", tm);
    printf(" %s ", time_str);

    clock_gettime(CLOCK_MONOTONIC, &tp);

    uint64_t days = tp.tv_sec / 86400;
    if (days != 0) {
        printf("%d day", days);
        if (days > 1) {
            printf("s");
        }
        printf(", ");
        tp.tv_sec %= 86400;
    }

    uint64_t hours = tp.tv_sec / 3600;
    if (hours != 0) {
        printf("%d hour", hours);
         if (hours > 1) {
            printf("s");
        }
        printf(", ");
        tp.tv_sec %= 3600;
    }

    uint64_t minutes = tp.tv_sec / 60;
    if (minutes != 0) {
        printf("%d minute", minutes);
         if (minutes > 1) {
            printf("s");
        }
        printf(", ");
        tp.tv_sec %= 60;
    }

    printf("%d seconds\n", tp.tv_sec);
    return 0;
}
