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
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <grp.h>
#include <utmpx.h>
#include <string.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    struct passwd *pwd;
    char user[20];

    puts("Available users:");
    while ((pwd = getpwent())) {
        printf("%s ", pwd->pw_name);
    }
    puts("");

    while (1) {
        printf("\nusername: ");
        fflush(stdout);
        scanf("%s", user);
        fflush(stdout);

        pwd = getpwnam(user);
        if (pwd == NULL) {
            puts("login: did not find login name");
        } else {
            struct stat buf;
            int fd = open("/etc/motd", O_RDONLY);
            if (fd != -1) {
                fstat(fd, &buf);
                char *contents = malloc(buf.st_size);
                read(fd, contents, buf.st_size);
                write(0, contents, buf.st_size);
                puts("");
                free(contents);
            }

            int child = fork();
            if (child == -1) {
                perror("login: could not fork");
                return 1;
            } else if (child == 0) {
                struct utmpx entry;
                entry.ut_type = USER_PROCESS;
                strcpy(entry.ut_user, pwd->pw_name);
                setutxent();
                if (pututxline(&entry) == NULL) {
                    perror("login: could not log login");
                }
                endutxent();

                setuid(pwd->pw_uid);
                setgid(pwd->pw_gid);
                setenv("HOME", pwd->pw_dir, 1);
                chdir(pwd->pw_dir);
                execl(pwd->pw_shell, pwd->pw_shell, "--login", NULL);
                return 1;
            } else {
                waitpid(child, NULL, 0);
            }
        }
    }
}
