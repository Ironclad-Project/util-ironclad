/*
    login.c: Login program.
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

int login_entrypoint(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    struct passwd *pwd;
    char user[20];

    puts("Available users:");
    while ((pwd = getpwent())) {
        printf("%s ", pwd->pw_name);
    }
    puts("");

    printf("\nusername: ");
    fflush(stdout);
    scanf("%s", user);
    fflush(stdout);

    pwd = getpwnam(user);
    if (pwd == NULL) {
        perror("login: did not find login name");
        return 1;
    }

    setuid(pwd->pw_uid);
    seteuid(pwd->pw_uid);
    setenv("HOME", pwd->pw_dir, 1);
    execl(pwd->pw_shell, pwd->pw_shell, "--login", NULL);
    return 1;
}