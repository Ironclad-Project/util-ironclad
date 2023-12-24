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

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: watch [options] ...");
                puts("");
                puts("Options:");
                puts("-h        Print this help message");
                puts("-v        Print version information");
                return 0;
            case 'v':
               puts("watch" VERSION_STR);
               return 0;
            default:
                goto END_WHILE;
        }
    }

END_WHILE:
    struct tm *timeinfo;
    time_t rawtime;
    pid_t child;
    char *timestr;
    struct winsize w;
    size_t headerlen, timelen;
    for (;;) {
        time(&rawtime);
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        timestr = asctime(timeinfo);

        ioctl(0, TIOCGWINSZ, &w);
        printf("\e[1;1H\e[2J\e[7m");
        headerlen = printf("Every 2.0s: \"%s", argv[optind]);
        for (int i = optind + 1; i < argc; i++) {
            headerlen += printf(" %s", argv[i]);
        }
        headerlen += printf("\"");
        timelen = strlen(timestr);
        for (size_t i = 0; i <= w.ws_col - headerlen - timelen; i++) {
            printf(" ");
        }
        printf("%s\e[0m\n", timestr);

        child = fork();
        if (child == 0) {
            execvp(argv[optind], argv + optind);
        }
        sleep(2);
    }
}
