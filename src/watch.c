/*
    watch.c: Watch commands over time.
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
#include <time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/syscall.h>

int main(int argc, char *argv[], char *envp[]) {
    int stop_on_fail          = 0;
    int do_exec               = 0;
    double seconds_for_update = 2.0;

    char c;
    while ((c = getopt (argc, argv, "hvexn:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: watch [options] ...");
                puts("");
                puts("Options:");
                puts("-h        Print this help message");
                puts("-v        Print version information");
                puts("-e        Stop updates on the first error");
                puts("-x        Execute directly instead of passing to sh");
                puts("-n <secs> Time to wait between updates, 2.0 by default");
                return 0;
            case 'v':
                puts("watch" VERSION_STR);
                return 0;
            case 'e':
                stop_on_fail = 1;
                break;
            case 'x':
                do_exec = 1;
                break;
            case 'n':
                seconds_for_update = atof(optarg);
                if (seconds_for_update == 0) {
                    fputs("watch: invalid interval specified\n", stderr);
                    return 1;
                }
                break;
            case '?':
                return 1;
            default:
                goto END_WHILE;
        }
    }

    if (optind == argc) {
        fputs("watch: no command specified\n", stderr);
        return 1;
    }

END_WHILE:
    struct tm *timeinfo;
    time_t rawtime;
    char *timestr;
    struct winsize w;
    size_t headerlen, timelen;

    size_t cmd_len = strlen(argv[optind]);
    for (int i = optind + 1; i < argc; i++) {
        cmd_len += 1 + strlen(argv[i]);
    }
    char *cmd_str = calloc(cmd_len, sizeof(char));
    strcat(cmd_str, argv[optind]);
    for (int i = optind + 1; i < argc; i++) {
        strcat(cmd_str, " ");
        strcat(cmd_str, argv[i]);
    }

    for (;;) {
        time(&rawtime);
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        timestr = asctime(timeinfo);

        ioctl(0, TIOCGWINSZ, &w);
        printf("\e[1;1H\e[2J\e[7m");
        headerlen = printf("Every %.1f: \"%s\"", seconds_for_update, cmd_str);
        timelen = strlen(timestr);
        for (size_t i = 0; i <= w.ws_col - headerlen - timelen; i++) {
            printf(" ");
        }
        printf("%s\e[0m\n", timestr);

        int wstatus;
        if (do_exec) {
            int ret, errno;

            size_t passed_argc, passed_envc;
            for (passed_argc = 0; argv[optind + passed_argc]; passed_argc++);
            for (passed_envc = 0; envp[passed_envc]; passed_envc++);

            size_t len = strlen(argv[optind]);
            SYSCALL7(SYSCALL_SPAWN, argv[optind], len, argv + optind, passed_argc,
               (uint64_t)envp, passed_envc, NULL);
            if (errno) {
               perror("watch: could not execute");
               return 1;
            }

            waitpid(ret, &wstatus, 0);
        } else {
            wstatus = system(cmd_str);
        }

        if (WEXITSTATUS(wstatus) != 0 && stop_on_fail) {
            goto CLEANUP;
        }

        struct timespec duration;
        duration.tv_sec  = (time_t)seconds_for_update;
        duration.tv_nsec = (long)(seconds_for_update / 1000000);
        nanosleep(&duration, NULL);
    }

CLEANUP:
    free(cmd_str);
}
