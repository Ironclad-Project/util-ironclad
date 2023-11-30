/*
    newgrp.c: Change group.
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
#include <commons.h>

int main(int argc, char *argv[]) {
    char c;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: newgrp [options] <group>");
                puts("");
                puts("Options:");
                puts("-h        Print this help message");
                puts("-v        Display version information.");
                return 0;
            case 'v':
               puts("newgrp" VERSION_STR);
               return 0;
            default:
                goto END_WHILE;
        }
    }

END_WHILE:
   if (optind >= argc) {
      fprintf(stderr, "newgrp: no group was passed!\n");
      return 1;
   }

   int gid;
   if (sscanf(argv[optind], "%d", &gid) != 1) {
      fprintf(stderr, "newgrp: %s is not a valid group, groups are passed as a numeric value\n", argv[optind]);
      return 1;
   }

   if (setgid(gid)) {
      perror("newgrp: could not set gid");
      return 1;
   }

   struct passwd *curr = getpwuid(getuid());
   if (curr == NULL) {
      fprintf(stderr, "newgrp: could not get current user info\n");
      return 1;
   }

   if (execl(curr->pw_shell, curr->pw_shell, NULL)) {
      fprintf(stderr, "newgrp: could not execute %s\n", curr->pw_shell);
      return 1;
   }
}
