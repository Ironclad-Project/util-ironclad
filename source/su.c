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
#include <commons.h>
#include <pwd.h>
#include <string.h>
#include <sys/reboot.h>

int main(int argc, char *argv[]) {
    int do_login_shell = 0;
    char *user = NULL;
    char **command = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            puts("Usage: su [options] [-] [username] [-c \"<command>\"]");
            puts("");
            puts("Options:");
            puts("-h               Print this help message");
            puts("-v               Display version information.");
            puts("-c \"<command>\" Instead of logging, execute a command in the shell");
            return 0;
        } else if (!strcmp(argv[i], "-v")) {
            puts("su" VERSION_STR);
            return 0;
        } else if (!strcmp(argv[i], "-")) {
            do_login_shell = 1;
        } else if (!strcmp(argv[i], "-c")) {
            command = &argv[i + 1];
            break;
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

    if (command != NULL) {
        size_t arg_count;
        for (arg_count = 0; command[arg_count] != NULL; arg_count += 1);

        // 1 more for pw_shell, 1 more for -c, 1 more for NULL.
        char **new_argvp = malloc(sizeof(char *) * (arg_count + 3));
        if (new_argvp == NULL) {
            perror("su: could not allocate new argvp string");
            return 1;
        }

        new_argvp[0] = strdup(pwd->pw_shell);
        new_argvp[1] = strdup("-c");
        for (size_t i = 0; i < arg_count; i++) {
            new_argvp[i + 2] = strdup(command[i]);
        }
        new_argvp[arg_count + 2] = NULL;

        execvp(pwd->pw_shell, new_argvp);
    } else if (do_login_shell) {
        setenv("HOME", pwd->pw_dir, 1);
        execl(pwd->pw_shell, pwd->pw_shell, "--login", NULL);
    } else {
        execl(pwd->pw_shell, pwd->pw_shell, NULL);
    }
    return 1;
}
