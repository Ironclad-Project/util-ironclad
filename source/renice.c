/*
    renice.c: Change niceness of other processes.
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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <commons.h>
#include <sys/resource.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int increment = 0;
    int setpid = 0;
    int pid = 0;
    int setpgid = 0;
    int pgid = 0;
    int setuid = 0;
    int uid = 0;

    char c;
    while ((c = getopt (argc, argv, "hvn:g:p:u:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: renice -n <increment> [-g|-p|-u] id");
                puts("");
                puts("Options:");
                puts("-h             Print this help message");
                puts("-v             Display version information.");
                puts("-n <increment> Increment to set to niceness");
                puts("-g <pgid>      Set niceness to the passed group");
                puts("-p <pid>       Set niceness to the passed pid");
                puts("-u <uid>       Set niceness to the passed uid");
                return 0;
            case 'v':
               puts("renice" VERSION_STR);
               return 0;
            case 'n':
               if (sscanf(optarg, "%d", &increment) != 1) {
                   fprintf(stderr, "renice: '%s' is not a valid increment", optarg);
               }
               break;
            case 'g':
               setpgid = 1;
               if (sscanf(optarg, "%d", &pgid) != 1) {
                   fprintf(stderr, "renice: '%s' is not a valid PGID", optarg);
               }
               break;
            case 'p':
               setpid = 1;
               if (sscanf(optarg, "%d", &pid) != 1) {
                   fprintf(stderr, "renice: '%s' is not a valid PID", optarg);
               }
               break;
            case 'u':
               setuid = 1;
               if (sscanf(optarg, "%d", &uid) != 1) {
                   fprintf(stderr, "renice: '%s' is not a valid UID", optarg);
               }
               break;
            default:
                fprintf(stderr, "renice: Unknown option '%c'\n", optopt);
                return 1;
        }
    }

    int which, who;
    if (setpgid) {
        which = PRIO_PGRP;
        who = pgid;
    } else if (setuid) {
        which = PRIO_USER;
        who = uid;
    } else if (setpid) {
        which = PRIO_PROCESS;
        who = pid;
    } else {
       fprintf(stderr, "renice: No entity to set niceness of\n");
       return 1;
    }

    int prio = getpriority(which, who);
    if (errno) {
        perror("renice: could not get priority");
        return 1;
    }
    if (setpriority(which, who, prio + increment)) {
        perror("renice: could not set priority");
        return 1;
    }

    return 0;
}
