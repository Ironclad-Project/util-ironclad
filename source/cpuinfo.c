/*
    cpuinfo.c: Display CPU information.
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

int cpuinfo_entrypoint(int argc, char *argv[]) {
    bool only_name  = false;
    bool only_cores = false;
    bool only_freq  = false;

    char c;
    while ((c = getopt (argc, argv, "hncf")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: cpuinfo [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-n              Display only the name of the CPU.");
                puts("-c              Display only the count of cores.");
                puts("-f              Display only the frequency of the CPU.");
                puts("-v | --version  Display version information.");
                return 0;
            case 'n': only_name  = true; break;
            case 'c': only_cores = true; break;
            case 'f': only_freq  = true; break;
            default:
                fputs("cpuinfo: Unrecognized option\n", stderr);
                return 1;
        }
    }

    // Fetch name.
    char name[48];
    __cpuid(0x80000002, *(uint32_t *)(name +  0), *(uint32_t *)(name +  4), *(uint32_t *)(name +  8), *(uint32_t *)(name + 12));
    __cpuid(0x80000003, *(uint32_t *)(name + 16), *(uint32_t *)(name + 20), *(uint32_t *)(name + 24), *(uint32_t *)(name + 28));
    __cpuid(0x80000004, *(uint32_t *)(name + 32), *(uint32_t *)(name + 36), *(uint32_t *)(name + 40), *(uint32_t *)(name + 44));

    // Fetch core count.
    uint32_t eax, ebx, ecx, edx, logical_cores;
    __cpuid(1, eax, ebx, ecx, edx);
    if ((edx & (1 << 28)) != 0) {
        logical_cores = (ebx >> 16) & 0xff;
    } else {
        logical_cores = 1;
    }

    // Fetch frequency.
    // TODO: Placeholder.
    uint32_t frequency = 3;

    if (only_name) {
        puts(name);
    } else if (only_cores) {
       printf("%d\n", logical_cores);
    } else if (only_freq) {
       printf("%d GHz\n", frequency);
    }
    return 0;
}
