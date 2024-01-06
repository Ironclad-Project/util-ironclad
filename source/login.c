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
#include <gcrypt.h>
#include <termios.h>

static const char *issue_path = "/etc/issue"; // To display before login.
static const char *motd_path = "/etc/motd";   // To display after login.

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

static const char * const hextable = "0123456789abcdef";
static void convert(unsigned char *src, char *out, int len) {
    while(len--) {
        *out++ = hextable[*src >> 4];
        *out++ = hextable[*src & 0x0F];
        src++;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    struct passwd *pwd;
    char user[64];

    print_whole_file(issue_path);

    while (1) {
        printf("\nusername: ");
        fflush(stdout);
        scanf("%s", user);
        fflush(stdout);

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

            char requested[64];
            char stringrequest[64 * 2];
            gcry_md_hash_buffer(GCRY_MD_SHA512, requested, user, strlen(user));
            convert(requested, stringrequest, 64);

            if (strncmp(pwd->pw_passwd, stringrequest, 64 * 2)) {
                puts("login: password did not match!");
                continue;
            }
        }

        puts("");
        print_whole_file(motd_path);

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
