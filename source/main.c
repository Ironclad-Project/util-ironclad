/*
    main.c: Entrypoint of the project
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
#include <string.h>
#include <entrypoints.h>
#include <stdbool.h>

static bool is_program(const char *invoked_name, size_t length,
                       const char *program) {
  size_t program_length = strlen(program);

  return (length >= program_length) &&
         !strcmp(invoked_name + (length - program_length), program);
}

int main(int argc, char *argv[]) {
    const char *invoked_name = argv[0];

    // Generalize version handling, the rest is utility-dependent.
    for (int i = 1; argv[i] != NULL; i++) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s from util-ironclad %s\n", invoked_name, PACKAGE_VERSION);
            puts("Copyright (C) 2023 streaksu.");
            puts("Licensed under the GNU GPL version 3 or later.");
            puts("This is free software: you are free to change and redistribute it.");
            puts("There is NO WARRANTY, to the extent permitted by law.");
            return 0;
        }
    }

    // Dispatch to the utilities.
    size_t invoked_length = strlen(invoked_name);
    if (is_program(invoked_name, invoked_length, "cpuinfo")) {
        return cpuinfo_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "execmac")) {
        return execmac_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "integrity")) {
        return integrity_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "mount")) {
        return mount_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "ps")) {
        return ps_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "showmem")) {
        return showmem_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "strace")) {
        return strace_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "umount")) {
        return umount_entrypoint(argc, argv);
    }  else if (is_program(invoked_name, invoked_length, "login")) {
        return login_entrypoint(argc, argv);
    } else if (is_program(invoked_name, invoked_length, "su")) {
        return su_entrypoint(argc, argv);
    } else {
        puts("Hello! this is util-ironclad");
        puts("util-ironclad provides some utilities in a single-executable");
        puts("format in order to save executable space. Each utility is a");
        puts("symlink to the global one. The implemented utilities are:");
        puts("cpuinfo execmac integrity mount showmem umount");
        return 0;
    }
}
