/*
    commons.h: Common things between programs.
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

#pragma once

#include <stdint.h>

#define VERSION_STR                                                       \
   " provided by util-ironclad " PACKAGE_VERSION "\n"                     \
   "Copyright (C) 2023 streaksu.\n"                                       \
   "License under the GNU GPL <https://gnu.org/licenses/gpl.html>.\n"     \
   "This is free software: you are free to change and redistribute it.\n" \
   "There is NO WARRANTY, to the extent permitted by law."

typedef struct {
    uint32_t  time_low;
    uint16_t  time_mid;
    uint16_t  time_hi_and_version;
    uint8_t   clock_seq_hi_and_reserved;
    uint8_t   clock_seq_low;
    uint8_t   node[6];
} uuid_t;

#define UUID_STR_LEN 36
