/*
    blkid.c: Display UUIDs.
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

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons.h>
#include <utmpx.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>

#define DEV_UUID 0x9821

static void print_fd_uuid(const char *path) {
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("blkid: count not open device");
        return;
    }

    uuid_t uuid;
    if (ioctl(fd, DEV_UUID, &uuid)) {
        perror("blkid: count not fetch uuid of device");
        return;
    }

    close(fd);

    uuid_t zero_uuid = {0};
    if (memcmp(&uuid, &zero_uuid, sizeof(uuid_t)) == 0) {
        return;
    }

    printf("%s: %8.8x-%4.4x-%4.4x-%2.2x%2.2x-", path,
           uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
           uuid.clock_seq_hi_and_reserved, uuid.clock_seq_low);
    for (int i = 0; i < 6; i++) {
        printf("%2.2x", uuid.node[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: uptime [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("blkid" VERSION_STR);
               return 0;
            default:
                break;
        }
    }

    if (optind < argc) {
        for(; optind < argc; optind++) {
            print_fd_uuid(argv[optind]);
        }
    } else {
        DIR *folder = opendir("/dev");
        if (folder == NULL) {
            perror("blkid: count not open /dev");
            return 1;
        }

        struct dirent *entry;
        char path[1025];
        while ((entry = readdir(folder))) {
            if (entry->d_name[0] != '.') {
                strcpy(path, "/dev/");
                strcat(path, entry->d_name);
                print_fd_uuid(path);
            }
        }

        closedir(folder);
   }

   return 0;
}
