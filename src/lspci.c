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

#define SC_LIST_PCI 21

struct devinfo {
    uint8_t  bus;
    uint8_t  func;
    uint8_t  slot;
    uint16_t device_id;
    uint16_t vendor_id;
    uint8_t  rev_id;
    uint8_t  subclass;
    uint8_t  device_class;
    uint8_t  prog_if;
};

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: lspci [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("lspci" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "lspci: %c is not a valid argument\n", optopt);
                return 1;
        }
    }

    int ret, errno;
    struct devinfo *buffer = malloc(20 * sizeof(struct devinfo));
    if (!buffer) {
        perror("lspci: could not allocate buffer");
        return 1;
    }

    SYSCALL3(SYSCALL_SYSCONF, SC_LIST_PCI, buffer, 20 * sizeof(struct devinfo));
    if (ret > 20) {
        buffer = realloc(buffer, ret * sizeof(struct devinfo));
        if (!buffer) {
            perror("lspci: could not allocate buffer");
            return 1;
        }
        SYSCALL3(SYSCALL_SYSCONF, SC_LIST_PCI, buffer, ret * sizeof(struct devinfo));
    }

    if (ret == -1) {
        perror("lspci: could not fetch devices");
        return 1;
    } else if (ret == 0) {
        return 1;
    }

    for (int i = 0; i < ret; i++) {
        printf("%02x:%02x:%02x Unknown device (%02x:%02x:%02x) (rev %x)\n",
               buffer[i].bus, buffer[i].slot, buffer[i].func,
               buffer[i].device_class, buffer[i].subclass, buffer[i].prog_if,
               buffer[i].rev_id);
    }

    return 0;
}
