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
    if (!strcmp(invoked_name, "cpuinfo")) {
        return cpuinfo_entrypoint(argc, argv);
    } else if (!strcmp(invoked_name, "mount")) {
        return mount_entrypoint(argc, argv);
    } else if (!strcmp(invoked_name, "showmem")) {
        return showmem_entrypoint(argc, argv);
    } else if (!strcmp(invoked_name, "umount")) {
        return umount_entrypoint(argc, argv);
    } else {
        puts("Hello! this is util-ironclad");
        puts("util-ironclad provides some system utilities in a single-executable");
        puts("format in order to save executable space. Each utility is a");
        puts("symlink to the global one. The implemented utilities are:");
        puts("mount showmem umount");
        return 0;
    }
}
