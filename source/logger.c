/*
    logger.c: Log strings.
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
#include <stddef.h>
#include <unistd.h>
#include <sys/mac.h>
#include <string.h>
#include <stdlib.h>
#include <commons.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    size_t total_size = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            puts("Usage: logger [options] ...");
            puts("");
            puts("Options:");
            puts("-h            Print this help message");
            puts("-v            Print version information");
            return 0;
        } else if (!strcmp(argv[i], "-v")) {
            puts("logger" VERSION_STR);
            return 0;
        } else {
            total_size += strlen(argv[i]) + 1;
        }
    }

    char *total_string = calloc(1, total_size + 1);
    for (int i = 1; i < argc; i++) {
        strcat(total_string, argv[i]);
        strcat(total_string, " ");
    }
    total_string[total_size - 1] = '\0';

    openlog("logger", 0, LOG_USER);
    syslog(LOG_INFO, "%s", total_string);
    closelog();
    free(total_string);
}
