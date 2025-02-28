/*
    ifconfig.c: Network interface configuration.
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

#define SC_LIST_NETINTER 14
#define NETINTER_BLOCKED 0b1

struct netinterface {
    char devname[65];
    uint64_t flags;
    uint8_t mac_addr[6];
    uint8_t ipv4_addr[4];
    uint8_t ipv4_subnet[4];
    uint8_t ipv6_addr[16];
    uint8_t ipv6_subnet[16];
} __attribute__((packed));

static int ipv6addr_is_not_zero(uint8_t *addr) {
    for (int i = 0; i < 16; i++) {
        if (addr[i] != 0) return 1;
    }
    return 0;
}

static void print_ipv6addr(uint8_t *addr) {
    int skipped_0s = 0;
    for (int i = 0; i < 16; i++) {
        if (addr[i] != 0) {
            if (skipped_0s) {
                printf(":");
            }
            if (i != 0) {
               printf(":");
            }
            printf("%x", addr[i]);
            if (i != 15) {
                printf(":");
            }
        } else {
            skipped_0s = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int do_block = 0;
    int do_unblock = 0;
    char *blocked = NULL;

    char c;
    while ((c = getopt(argc, argv, "hb:u:v")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ifconfig");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-b <name>       Block the passed interface");
                puts("-u <name>       Unblock the passed interface");
                puts("-v              Display version information.");
                return 0;
            case 'b':
                do_block = 1;
                blocked = strdup(optarg);
                break;
            case 'u':
                do_unblock = 1;
                blocked = strdup(optarg);
                break;
            case 'v':
               puts("ifconfig" VERSION_STR);
               return 0;
            default:
                if (optopt == 'b') {
                    fprintf(stderr, "ifconfig: %c needs an argument\n", optopt);
                } else {
                    fprintf(stderr, "ifconfig: %c is not a valid argument\n", optopt);
                }
                return 1;
        }
    }

    if (do_block || do_unblock) {
        int ret, errno;
        bool actually_block = !!do_block;
        char path[64];
        snprintf(path, 64, "/dev/%s", blocked);
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "ifconfig: could not open %s for blocking\n", path);
            return 1;
        }
        SYSCALL3(SYSCALL_CONFIG_NETINTER, fd, 1, &actually_block);
        return errno;
    }

    struct netinterface *buffer = malloc(10 * sizeof(struct netinterface));
    long ret, errno;
    SYSCALL2(SYSCALL_LISTNETINTER, buffer, 10);
    if (ret == -1) {
        return 1;
    }

    for (int i = 0; i < ret; i++) {
        printf("%s: <%s>\n", buffer[i].devname, buffer[i].flags & NETINTER_BLOCKED ? "BLOCKED" : "UNBLOCKED");
        printf("\tether %02x:%02x:%02x:%02x:%02x:%02x\n",
               buffer[i].mac_addr[0], buffer[i].mac_addr[1],
               buffer[i].mac_addr[2], buffer[i].mac_addr[3],
               buffer[i].mac_addr[4], buffer[i].mac_addr[5]);
         printf("\tinet %d.%d.%d.%d netmask %d.%d.%d.%d\n",
               buffer[i].ipv4_addr[0], buffer[i].ipv4_addr[1],
               buffer[i].ipv4_addr[2], buffer[i].ipv4_addr[3],
               buffer[i].ipv4_subnet[0], buffer[i].ipv4_subnet[1],
               buffer[i].ipv4_subnet[2], buffer[i].ipv4_subnet[3]);
         if (ipv6addr_is_not_zero(buffer[i].ipv6_addr)) {
             printf("\tinet6 ");
             print_ipv6addr(buffer[i].ipv6_addr);
             printf(" netmask ");
             print_ipv6addr(buffer[i].ipv6_addr);
             printf("\n");
         }
    }

    return 0;
}
