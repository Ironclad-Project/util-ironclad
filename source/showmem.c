/*
    showmem.c: Display memory statistics.
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
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <commons.h>

#define SC_PAGESIZE      1
#define SC_AVPHYS_PAGES  4
#define SC_PHYS_PAGES    5
#define SC_TOTAL_PAGES   7

int main(int argc, char *argv[]) {
    int print_only_free      = 0;
    int print_only_used      = 0;
    int print_only_available = 0;
    int print_only_installed = 0;

    char c;
    while ((c = getopt (argc, argv, "hfutv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: showmem [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-f              Print free memory (in MiB)");
                puts("-u              Print used memory (in MiB)");
                puts("-t              Print available memory (in MiB)");
                puts("-i              Print total installed memory (in MiB)");
                puts("-v              Display version information.");
                return 0;
            case 'f': print_only_free      = 1; break;
            case 'u': print_only_used      = 1; break;
            case 't': print_only_available = 1; break;
            case 'i': print_only_installed = 1; break;
            case 'v':
               puts("showmem" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "showmem: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    long page_size, free_pages, available_pages, total_pages, ret, errno;
    SYSCALL1(SYSCALL_SYSCONF, SC_PAGESIZE);
    if (ret == -1) {
        return 1;
    } else {
        page_size = ret;
    }
    SYSCALL1(SYSCALL_SYSCONF, SC_AVPHYS_PAGES);
    if (ret == -1) {
        return 1;
    } else {
        free_pages = ret;
    }
    SYSCALL1(SYSCALL_SYSCONF, SC_PHYS_PAGES);
    if (ret == -1) {
        return 1;
    } else {
        available_pages = ret;
    }
    SYSCALL1(SYSCALL_SYSCONF, SC_TOTAL_PAGES);
    if (ret == -1) {
        return 1;
    } else {
        total_pages = ret;
    }

    long free      = (free_pages      * page_size) / 1000000;
    long available = (available_pages * page_size) / 1000000;
    long total     = (total_pages     * page_size) / 1000000;
    int width      = round(1 + log(total) / log(10));

    if (print_only_free)           { printf("%lu\n", free);                 }
    else if (print_only_used)      { printf("%lu\n", available - free);     }
    else if (print_only_available) { printf("%lu\n", available);            }
    else if (print_only_installed) { printf("%lu\n", total);                }
    else {
        printf("Free physical memory:      %*luMiB\n", width, free);
        printf("Used physical memory:      %*luMiB\n", width, available - free);
        printf("Available physical memory: %*luMiB\n", width, available);
        printf("Total physical memory:     %*luMiB\n", width, total);
    }

   return 0;
}
