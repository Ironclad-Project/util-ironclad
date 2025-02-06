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
#include <crypt.h>
#include <termios.h>
#include <sys/time.h>

static const char *issue_path = "/etc/issue"; // To display before login.
static const char *motd_path  = "/etc/motd";   // To display after login.

static void print_whole_file(const char *path) {
    struct stat buf;
    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        fstat(fd, &buf);
        char *contents = malloc(buf.st_size);
        read(fd, contents, buf.st_size);
        write(0, contents, buf.st_size);
        puts("");
        free(contents);
        close(fd);
    }
}

#define USER_LEN 64

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    struct passwd *pwd;
    char user[USER_LEN + 1];

    print_whole_file(issue_path);

    while (1) {
        printf("username: ");
        fflush(stdout);
        fgets(user, USER_LEN, stdin);
        fflush(stdout);
        user[strcspn(user, "\n")] = '\0';

        if (strlen(user) == 0) {
            continue;
        }

        pwd = getpwnam(user);
        if (pwd == NULL) {
            puts("login: did not find login name");
            continue;
        }

        if (strcmp(pwd->pw_passwd, "")) {
            if (strlen(pwd->pw_passwd) != 64 * 2) {
                puts("login: user entry is invalid");
                continue;
            }

            printf("password (no echo): ");
            fflush(stdout);

            struct termios tp, save;
            tcgetattr(STDIN_FILENO, &tp);
            save = tp;
            tp.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp);

            scanf("%s", user);
            fflush(stdout);

            tcsetattr(STDIN_FILENO, TCSANOW, &save);

            char *requested = crypt(user, "GL");

            if (strcmp(pwd->pw_passwd, requested)) {
                puts("login: password did not match!");
                continue;
            }
        }

        puts("");
        print_whole_file(motd_path);

        pid_t login_pid = getpid();
        pid_t     child = fork();
        char       *tty = ttyname(0);

        if (child == -1) {
            perror("login: could not fork");
            return 1;
        } else if (child == 0) {
            struct utmpx entry;
            entry.ut_type = USER_PROCESS;
            strcpy(entry.ut_user, pwd->pw_name);
            gettimeofday(&entry.ut_tv, NULL);
            entry.ut_pid = login_pid;
            strcpy(entry.ut_user, pwd->pw_name);
            strcpy(entry.ut_line, tty);
            entry.ut_id[0] = '\0';

            setutxent();
            if (pututxline(&entry) == NULL) {
                perror("login: could not log login");
            }
            endutxent();

            setuid(pwd->pw_uid);
            setgid(pwd->pw_gid);
            setenv("HOME", pwd->pw_dir, 1);
            setenv("LOGNAME", pwd->pw_name, 1);
            chdir(pwd->pw_dir);
            execl(pwd->pw_shell, pwd->pw_shell, "--login", NULL);
            return 1;
        } else {
            waitpid(child, NULL, 0);
        }
    }
}
