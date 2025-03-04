/*
    showmem.c: Display memory statistics.
    Copyright (C) 2024 streaksu

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

struct mem_info {
    // All data is in bytes.
    uint64_t phys_total;     // Total physical memory of the system.
    uint64_t phys_available; // Non-reserved memory managed by the system.
    uint64_t phys_free;      // Free memory available to the system.
    uint64_t shared_usage;   // Amount of shared memory in the system.
    uint64_t kernel_usage;   // Amount of memory in use by the kernel.
    uint64_t table_usage;    // Of the kernel, amount in use for page tables.
    uint64_t poison_usage;   // Faulty memory.
};

int main(int argc, char *argv[]) {
    int print_only_free  = 0;
    int print_only_used  = 0;
    int print_only_avail = 0;
    int print_only_total = 0;

    char c;
    while ((c = getopt (argc, argv, "hfutv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: showmem [options]");
                puts("");
                puts("Options:");
                puts("-h      Print this help message");
                puts("-f      Print free memory (in MiB)");
                puts("-u      Print used memory (in MiB)");
                puts("-t      Print available memory (in MiB)");
                puts("-i      Print total installed system memory (in MiB)");
                puts("-v      Display version information.");
                return 0;
            case 'f': print_only_free  = 1; break;
            case 'u': print_only_used  = 1; break;
            case 't': print_only_avail = 1; break;
            case 'i': print_only_total = 1; break;
            case 'v':
               puts("showmem" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "showmem: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    int ret, errno;
    struct mem_info meminfo;
    SYSCALL1(SYSCALL_MEMINFO, &meminfo);
    if (ret != 0) {
        return 1;
    }

    // Translate all values to kilobytes.
    const long free       = meminfo.phys_free      / 1000;
    const long available  = meminfo.phys_available / 1000;
    const long total      = meminfo.phys_total     / 1000;

    if (print_only_free)       { printf("%lu\n", free / 1000);               }
    else if (print_only_used)  { printf("%lu\n", (available - free) / 1000); }
    else if (print_only_avail) { printf("%lu\n", available / 1000);          }
    else if (print_only_total) { printf("%lu\n", total / 1000);              }
    else {
        const int width = round(1 + log(total) / log(10));

        printf("Free memory:      %*lu kB\n", width, free);
        printf("Used memory:      %*lu kB\n", width, available - free);
        printf("Available memory: %*lu kB\n", width, available);
        printf("Total memory:     %*lu kB\n", width, total);
        printf("Shared memory:    %*lu kB\n", width, meminfo.shared_usage / 1000);
        printf("Kernel memory:    %*lu kB\n", width, meminfo.kernel_usage / 1000);
        printf("Table memory:     %*lu kB\n", width, meminfo.table_usage  / 1000);
        printf("Poisoned memory:  %*lu kB\n", width, meminfo.poison_usage / 1000);
    }

   return 0;
}
