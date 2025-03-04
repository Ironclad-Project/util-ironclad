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

#if defined(__x86_64__)
   #include <cpuid.h>
#endif

struct cpuinfo {
    uint64_t conf_cores;
    uint64_t onln_cores;
    char model_name[64];
    char vendor_name[64];
    uint32_t base_mhz;
    uint32_t max_mhz;
    uint32_t ref_mhz;
};

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

    // Fetch CPU information by using getcpuinfo.
    struct cpuinfo cpu;
    int ret, errno;
    SYSCALL1(SYSCALL_GETCPUINFO, &cpu);
    if (ret == -1) {
        perror("Could not get CPU information from the kernel");
        return 1;
    }

    // Fetch base frequency in GHz.
    double base_frequency = ((double)cpu.base_mhz) / 1000;
    double max_frequency = ((double)cpu.max_mhz) / 1000;
    double reference_frequency = ((double)cpu.ref_mhz) / 1000;

    #if defined(__x86_64__)
        uint32_t eax, ebx, ecx, edx;
        if (__get_cpuid(0x16, &eax, &ebx, &ecx, &edx)) {
            base_frequency = ((double)eax) / 1000;
            max_frequency = ((double)ebx) / 1000;
            reference_frequency = ((double)ecx) / 1000;
        }
    #endif

    if (only_name) {
        printf("%.*s\n", (int)strnlen(cpu.model_name, 64), cpu.model_name);
    } else if (only_cores) {
       printf("%ld\n", cpu.conf_cores);
    } else if (only_freq) {
       printf("%f GHz\n", base_frequency);
    } else {
        printf("Vendor ID:          %.*s\n", (int)strnlen(cpu.vendor_name, 64), cpu.vendor_name);
        printf("Model name:         %.*s\n", (int)strnlen(cpu.model_name, 64), cpu.model_name);
        printf("CPU Base Freq:      %f GHz\n", base_frequency);
        printf("CPU Max Freq:       %f GHz\n", max_frequency);
        printf("CPU Reference Freq: %f GHz\n", reference_frequency);
        printf("Cores per socket:   %ld\n", cpu.conf_cores);
    }
}
