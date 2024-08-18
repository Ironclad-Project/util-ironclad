/*
    lp.c: Line printer fun.
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <math.h>
#include <commons.h>
#include <time.h>
#include <errno.h>

#define OUTPUT_WIDTH        80
#define MIN_DATE_SEPARATION 3

void print_to(FILE *source, FILE *desto) {
    char c = fgetc(source);
    while (c != EOF) {
        fputc(c, desto);
        c = fgetc(source);
    }
}

int main(int argc, char *argv[]) {
    FILE *dest = NULL;
    char *title = NULL;

    char c;
    while ((c = getopt (argc, argv, "hvcd:n:t:")) != -1) {
        switch (c) {
            case 'h':
                puts("Usage: lp [options] [file...]");
                puts("");
                puts("Options:");
                puts("-h        Print this help message");
                puts("-v        Display version information.");
                puts("-c        Exit after access is no longer needed, else, exit when writes are issued");
                puts("-d <dest> Specify a destination, else, let lp decide!");
                puts("-n <num>  Number of copies to print");
                puts("-t <titl> Title to add to the files");
                return 0;
            case 'v':
               puts("lp" VERSION_STR);
               return 0;
            case 'c':
               //  TODO.
               break;
            case 'd':
               dest = fopen(optarg, "w+");
               if (dest == NULL) {
                   perror("lp: could not open destination");
                   return 1;
               }
               break;
            case 't':
               title = strdup(optarg);
               break;
            default:
                if (optopt == 'd' || optopt == 'n' || optopt == 't') {
                    fputs("lp: Option requires an argument\n", stderr);
                    return 1;
                } else {
                    goto END_WHILE;
                }
                break;
        }
    }

END_WHILE:
    if (dest == NULL) {
        dest = fopen("/dev/lpt1", "w+");
        if (dest == NULL) {
            perror("lp: could not open default destination");
            return 1;
        }
    }

    if (title != NULL) {
       time_t t = time(NULL);
       struct tm *tm = localtime(&t);
       char timestr[64];
       size_t timestr_len = strftime(timestr, sizeof(timestr), "%c", tm);

       size_t title_len = strlen(title);
       size_t max_length = OUTPUT_WIDTH - timestr_len - MIN_DATE_SEPARATION;
       if (title_len > max_length) {
          fprintf(dest, "%.*s...   ", (unsigned int)max_length - 3, title);
       } else {
          fprintf(dest, "%-*s   ", (unsigned int)max_length, title);
       }

       fprintf(dest, "%s\n", timestr);
       for (size_t i = 0; i < OUTPUT_WIDTH; i++) {
           fputc('-', dest);
       }
       fputc('\n', dest);
    }

    if (optind < argc) {
      for(; optind < argc; optind++){
          if (!strcmp(argv[optind], "-")) {
              print_to(stdin, dest);
          } else {
              FILE *fp = fopen(argv[optind], "r");
              if (fp == NULL) {
                  fprintf(stderr, "lp: could not open input file %s: %s\n", argv[optind], strerror(errno));
              }
              print_to(fp, dest);
          }
      }
    } else {
        print_to(stdin, dest);
    }

    return 0;
}
