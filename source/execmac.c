/*
    execmac.c: Execute a program with certain MAC permissions.
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
#include <stddef.h>
#include <unistd.h>
#include <sys/ironclad_mac.h>
#include <string.h>
#include <sys/wait.h>

int execmac_entrypoint(int argc, char *argv[]) {
    char    *capability;
    uint64_t translated_caps;
    size_t filt_len;
    struct mac_filter filt;

    int c;
    while ((c = getopt(argc, argv, "hc:f:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: execmac [options] [command]");
                puts("");
                puts("Options:");
                puts("-h            Print this help message");
                puts("-v|--version  Print version information");
                puts("-c            Capabilities to give the command");
                puts("-f            Add a file filter");
                puts("");
                puts("Command:");
                puts("Command that will be run using the passed MAC settings");
                puts("");
                puts("Available capabilities for \"-c cap1,cap2,cap3\":");
                puts("sched:   Can change its own scheduling rules.");
                puts("spawn:   Can create other processes.");
                puts("entropy: Can access entropy-related syscalls.");
                puts("mem:     Can allocate and deallocate global memory.");
                puts("usenet:  Can use network.");
                puts("sysnet:  Can configure networking system-wide.");
                puts("mnt:     Can mount/unmount.");
                puts("pwr:     Can modify power levels and reboot/shutdown.");
                puts("all:     Use all capabilities, must be passed alone.");
                return 0;
            case 'c':
                if (!strncmp(optarg, "all", 3)) {
                    translated_caps = (uint64_t)-1;
                    break;
                } else {
                    translated_caps = 0;
                }

                capability = strtok(optarg, ",");
                while (capability != NULL) {
                    if (!strncmp(capability, "sched", 5)) {
                        translated_caps |= MAC_CAP_SCHED;
                    } else if (!strncmp(capability, "spawn", 5)) {
                        translated_caps |= MAC_CAP_SPAWN;
                    } else if (!strncmp(capability, "entropy", 7)) {
                        translated_caps |= MAC_CAP_ENTROPY;
                    } else if (!strncmp(capability, "mem", 3)) {
                        translated_caps |= MAC_CAP_SYS_MEM;
                    } else if (!strncmp(capability, "usenet", 6)) {
                        translated_caps |= MAC_CAP_USE_NET;
                    } else if (!strncmp(capability, "sysnet", 6)) {
                        translated_caps |= MAC_CAP_SYS_NET;
                    } else if (!strncmp(capability, "mnt", 3)) {
                        translated_caps |= MAC_CAP_SYS_MNT;
                    } else if (!strncmp(capability, "pwr", 3)) {
                        translated_caps |= MAC_CAP_SYS_PWR;
                    } else {
                        fputs("execmac: bad cap", stderr);
                        return 1;
                    }
                    capability = strtok(NULL, ",");
                }
                break;
            case 'f':
                filt_len = strlen(optarg);
                if (filt_len > 75) {
                    fputs("execmac: Filter path > 75", stderr);
                }
                strcpy(filt.path, optarg);
                filt.length = filt_len;
                filt.perms  = (uint8_t)-1;
                if (add_mac_filter(&filt)) {
                    perror("execmac: Could not set file filter");
                    return 1;
                }
                break;
            default:
                if (optopt == 'c' || optopt == 'f') {
                    fprintf(stderr, "execmac: %c needs an argument\n", optopt);
                    return 1;
                } else {
                    goto END_WHILE;
                }
                break;
        }
    }

END_WHILE:
    int child = fork();
    if (child == 0) {
        if (set_mac_capabilities(translated_caps)) {
            perror("execmac: Could not set MAC capabilities");
            return 1;
        }

        if (lock_mac()) {
            perror("execmac: Could not lock MAC");
            return 1;
        }

        if (execvp(argv[optind], argv + optind)) {
            perror("execmac: Could not launch program");
            return 1;
        }
    }

    int status;
    pid_t success = waitpid(child, &status, 0);
    if (success == -1) {
        perror("execmac: Could not wait for child");
        return 1;
    } else {
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return 1;
        }
    }
}