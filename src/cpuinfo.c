/*
    cpuinfo.c: Display CPU information.
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

int main(int argc, char *argv[]) {
    bool only_name  = false;
    bool only_cores = false;
    bool only_freq  = false;

    char c;
    while ((c = getopt (argc, argv, "hncfv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: cpuinfo [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-n              Display only the name of the CPU.");
                puts("-c              Display only the count of cores.");
                puts("-f              Display only the frequency of the CPU.");
                puts("-v              Display version information.");
                return 0;
            case 'n': only_name  = true; break;
            case 'c': only_cores = true; break;
            case 'f': only_freq  = true; break;
            case 'v':
               puts("cpuinfo" VERSION_STR);
               return 0;
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

    // Fetch vendor name.
    char vendor[12];
    uint32_t throwaway = 0;
    __cpuid(0x0, throwaway, *(uint32_t *)(&vendor[0] +  0), *(uint32_t *)(&vendor[0] +  8), *(uint32_t *)(&vendor[0] +  4));

    // Fetch core count.
    uint32_t eax, ebx, ecx, edx, logical_cores;
    __cpuid(1, eax, ebx, ecx, edx);
    if ((edx & (1 << 28)) != 0) {
        logical_cores = (ebx >> 16) & 0xff;
    } else {
        logical_cores = 1;
    }

    // Fetch frequency in GHz
    // TODO: Have a method for fetching it, these are placeholders.
    double base_frequency      = 3.0;
    double max_frequency       = 3.0;
    double reference_frequency = 3.0;

    if (only_name) {
        printf("%.*s\n", 48, name);
    } else if (only_cores) {
       printf("%d\n", logical_cores);
    } else if (only_freq) {
       printf("%f GHz\n", base_frequency);
    } else {
        printf("Vendor ID:          %.*s\n", 12, vendor);
        printf("Model name:         %.*s\n", 48, name);
        printf("CPU Base Freq:      %f GHz\n", base_frequency);
        printf("CPU Max Freq:       %f GHz\n", max_frequency);
        printf("CPU Reference Freq: %f GHz\n", reference_frequency);
        printf("Cores per socket:   %d\n", logical_cores);
    }
}