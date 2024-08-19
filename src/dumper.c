/*
    dumper.c: Dump corefiles.
    Copyright (C) 2024 streaksu

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <sys/mount.h>
#include <commons.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <inttypes.h>

struct registers {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

int main(int argc, char *argv[]) {
    char c;
    char *corefile = NULL;
    while ((c = getopt (argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: dumper [options] <filename>");
                puts("");
                puts("Options:");
                puts("-h Print this help message");
                puts("-v Display version information.");
                return 0;
            case 'v':
               puts("dumper" VERSION_STR);
               return 0;
            default:
                goto END_WHILE;
        }
    }

END_WHILE:
    for(; optind < argc; optind++){
        if (corefile == NULL) {
            corefile = strdup(argv[optind]);
        } else {
            fprintf(stderr, "dumper: Argument '%s' not used\n", argv[optind]);
        }
    }

    if (corefile == NULL) {
        fprintf(stderr, "dumper: No file for dumping was passed\n");
        return 1;
    }

    struct registers *contents = malloc(sizeof(struct registers));
    if (contents == NULL) {
        perror("dumper: could not allocate");
        return 1;
    }

    int core_fd = open(corefile, O_RDONLY);
    if (core_fd == -1) {
        perror("dumper: could not open passed file");
        return 1;
    }

    if (read(core_fd, contents, sizeof(struct registers)) != sizeof(struct registers)) {
        perror("dumper: could not read enough of the dump");
        return 1;
    }

    printf("Register dump at the time of fault (%" PRIx64 "):\n", contents->rip);
    printf("RAX: %016" PRIx64 " RBX: %016" PRIx64 " RCX: %016" PRIx64 "\n", contents->rax, contents->rbx, contents->rcx);
    printf("RDX: %016" PRIx64 " RSI: %016" PRIx64 " RDI: %016" PRIx64 "\n", contents->rdx, contents->rsi, contents->rdi);
    printf("RBP: %016" PRIx64 " R8:  %016" PRIx64 " R9:  %016" PRIx64 "\n", contents->rbp, contents->r8, contents->r9);
    printf("R10: %016" PRIx64 " R11: %016" PRIx64 " R12: %016" PRIx64 "\n", contents->r10, contents->r11, contents->r12);
    printf("R13: %016" PRIx64 " R14: %016" PRIx64 " R15: %016" PRIx64 "\n", contents->r13, contents->r14, contents->r15);
    printf("\n");
    printf("ERR: %" PRIx64 " CS: %" PRIx64 " RFLAGS: %" PRIx64 "\n", contents->err, contents->cs, contents->rflags);
    printf("RSP: %" PRIx64 " SS: %" PRIx64 "\n", contents->rsp, contents->ss);
}
