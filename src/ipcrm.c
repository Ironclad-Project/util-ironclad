/*
    ipcrm.c: Delete SystemV IPC.
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
    int do_all = 0;
    int remove_smhid = 0;

    char c;
    while ((c = getopt (argc, argv, "hvm:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: ipcrm [options]");
                puts("");
                puts("Options:");
                puts("-h       Print this help message");
                puts("-v       Display version information");
                puts("-m <id>  Mark for deletion shared memory segment by id");
                puts("-a       Mark all IPC everywhere for deletion");
                return 0;
            case 'v':
               puts("ipcrm" VERSION_STR);
               return 0;
            case 'm':
               if (sscanf(optarg, "%d", &remove_smhid) != 1) {
                   fprintf(stderr, "ipcrm: '%s' is not a valid shmid", optarg);
               }
               break;
            case 'a':
                do_all = 1;
                break;
            default:
                fprintf(stderr, "ipcrm: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    if (do_all) {
        // In Ironclad, shmids go from 1 to X, where X is 20 for current releases.
        // So we can just iterate that.
        for (int i = 1; i <= 20; i++) {
            shmctl(remove_smhid, IPC_RMID, NULL);
        }
    } else if (remove_smhid != 0) {
        if (shmctl(remove_smhid, IPC_RMID, NULL)) {
            fprintf(stderr, "ipcrm: Could not mark smhid %d for deletion\n", remove_smhid);
            return 1;
        }
    }

    return 0;
}
