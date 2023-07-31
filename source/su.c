/*
    su.c: Switch user program.
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
#include <entrypoints.h>
#include <pwd.h>
#include <string.h>
#include <sys/reboot.h>

int su_entrypoint(int argc, char *argv[]) {
    int do_login_shell = 0;
    char *user = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            puts("Usage: su [options] [-] [username]");
            puts("");
            puts("Options:");
            puts("-h              Print this help message");
            puts("-v | --version  Display version information.");
            return 0;
        } else if (!strcmp(argv[i], "-")) {
            do_login_shell = 1;
        } else {
            user = argv[i];
            break;
        }
    }

    if (user == NULL) {
       user = "root";
    }

    struct passwd *pwd = getpwnam(user);
    if (pwd == NULL) {
        fprintf(stderr, "su: user '%s' does not exist or the user entry is malformed\n", user);
        return 1;
    }

    setuid(pwd->pw_uid);
    seteuid(pwd->pw_uid);

    if (do_login_shell) {
        setenv("HOME", pwd->pw_dir, 1);
        execl(pwd->pw_shell, pwd->pw_shell, "--login", NULL);
    } else {
        execl(pwd->pw_shell, pwd->pw_shell, NULL);
    }
    return 1;
}
