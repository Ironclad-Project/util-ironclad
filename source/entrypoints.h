/*
    entrypoints.h: Entrypoints of all subprograms.
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

#pragma once

int cpuinfo_entrypoint(int argc, char *argv[]);
int klog_entrypoint(int argc, char *argv[]);
int execmac_entrypoint(int argc, char *argv[]);
int ifconfig_entrypoint(int argc, char *argv[]);
int mount_entrypoint(int argc, char *argv[]);
int ps_entrypoint(int argc, char *argv[]);
int showmem_entrypoint(int argc, char *argv[]);
int strace_entrypoint(int argc, char *argv[]);
int uptime_entrypoint(int argc, char *argv[]);
int login_entrypoint(int argc, char *argv[]);
int su_entrypoint(int argc, char *argv[]);
int umount_entrypoint(int argc, char *argv[]);
