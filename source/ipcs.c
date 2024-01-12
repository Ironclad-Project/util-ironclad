/*
    ipcs.c: Display SystemV IPC.
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
#include <sys/shm.h>
#include <commons.h>

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ipcs [options]");
                puts("");
                puts("Options:");
                puts("-h              Print this help message");
                puts("-v              Display version information.");
                return 0;
            case 'v':
               puts("ipcs" VERSION_STR);
               return 0;
            default:
                fprintf(stderr, "ipcs: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    // In Ironclad, shmids go from 1 to X, where X is 20 for current releases.
    // So we can just iterate that.
    struct shmid_ds buf;
    puts("Shared Memory Segments:");
    printf("%10s %5s %10s %10s %10s %6s\n", "key", "shmid", "owner", "perms",
        "bytes", "nattach");
    for (int i = 1; i <= 20; i++) {
        if (!shmctl(i, IPC_STAT, &buf)) {
            printf("%010d %5d %10d %10o %10zu %6ld\n", buf.shm_perm.__ipc_perm_key,
                i, buf.shm_perm.uid, buf.shm_perm.mode, buf.shm_segsz,
                buf.shm_nattch);
        }
    }
    return 0;
}
