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
#include <sys/mac.h>
#include <string.h>
#include <sys/wait.h>
#include <commons.h>

int main(int argc, char *argv[]) {
    char    *capability;
    uint64_t translated_caps = get_mac_capabilities();

    int c, ret;
    while ((c = getopt(argc, argv, "hvc:f:p:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: execmac [options] [command]");
                puts("");
                puts("Options:");
                puts("-h            Print this help message");
                puts("-v            Print version information");
                puts("-p            Policy to use, can be D, S, K");
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
                puts("ptrace:  Can use ptrace to trace children.");
                puts("setuid:  Can change the effective and real UID.");
                puts("mac:     Can modify MAC permissions and enforcement");
                puts("clock:   Can access clock resources");
                puts("killall: Can kill and signal to all processes");
                puts("setgid:  Can change process group ID");
                puts("ipc:     Can manage system IPC resources");
                puts("syslog:  Can access system logs, like dmesg");
                puts("all:     Use all capabilities, must be passed alone.");
                return 0;
            case 'v':
               puts("execmac" VERSION_STR);
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
                    } else if (!strncmp(capability, "ptrace", 6)) {
                        translated_caps |= MAC_CAP_PTRACE;
                    } else if (!strncmp(capability, "setuid", 6)) {
                        translated_caps |= MAC_CAP_SETUID;
                    } else if (!strncmp(capability, "mac", 3)) {
                        translated_caps |= MAC_CAP_SYS_MAC;
                    } else if (!strncmp(capability, "clock", 5)) {
                        translated_caps |= MAC_CAP_CLOCK;
                    } else if (!strncmp(capability, "killall", 7)) {
                        translated_caps |= MAC_CAP_SIGNALALL;
                    } else if (!strncmp(capability, "setgid", 6)) {
                        translated_caps |= MAC_CAP_SETGID;
                    } else if (!strncmp(capability, "ipc", 3)) {
                        translated_caps |= MAC_CAP_IPC;
                    } else if (!strncmp(capability, "syslog", 6)) {
                        translated_caps |= MAC_CAP_SYS_LOG;
                    } else {
                        fputs("execmac: bad cap", stderr);
                        return 1;
                    }
                    capability = strtok(NULL, ",");
                }
                break;
            case 'f':
                if (!strncmp(optarg, "/dev/", 5)) {
                    ret = add_mac_permissions(optarg + 5, 0b1011111);
                } else {
                    ret = add_mac_permissions(optarg, 0b11111);
                }

                if (ret) {
                    perror("execmac: Could not set file filter");
                    return 1;
                }
                break;
            case 'p':
                if (!strncmp(optarg, "D", 1)) {
                    set_mac_enforcement(MAC_DENY);
                } else if (!strncmp(optarg, "S", 1)) {
                    set_mac_enforcement(MAC_DENY_AND_SCREAM);
                } else if (!strncmp(optarg, "K", 1)) {
                    set_mac_enforcement(MAC_KILL);
                } else {
                    perror("execmac: Invalid policy");
                    return 1;
                }
                break;
            default:
                if (optopt == 'c' || optopt == 'f' || optopt == 'p') {
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

        if (execvp(argv[optind], argv + optind)) {
            perror("execmac: Could not launch program");
            return 1;
        }
    }

    int status;
    if (waitpid(child, &status, 0) == -1) {
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
