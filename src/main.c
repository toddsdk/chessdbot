/* Copyright (C) 2007-2008 Centro de Computacao Cientifica e Software Livre
 * Departamento de Informatica - Universidade Federal do Parana - C3SL/UFPR
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
ChessD BoT - A Free Chess Engine, intended to be used by children and teenagers
learning how to play Chess.

main.c
Main module. Contains the Main function and some functions of general purpose.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "main.h"
#include "cecp.h"
#include "search.h"
#include "levels.h"

static char *level_name = NULL;

/* Condition and mutex used to sync Search & Cecp Threads
 * (see search.c and cecp.c) */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {
    /* Threads atrributes */
    pthread_attr_t atrib_threads;
    /* Threads IDs (CECP and Search Threads) */
    pthread_t cecp_tid, search_tid;

    /* Turn off I/O buffers */
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    /* Initial message. Name and version */
    printf("ChessD BoT v0.2.2\n");

    /* Parse the command-line options */
    cmd_line_options(argc, argv);
    /* Validates level typed in command line */

    validate_level(level_name);
    /* Load the levels configuration file */
    load_levels(level_name);
    select_level("base_level");
    adjust_level(level_name);

    /* Set Threads as joinable */
    pthread_attr_init(&atrib_threads);
    pthread_attr_setdetachstate(&atrib_threads, PTHREAD_CREATE_JOINABLE);

    /* Start CECP Thread (I/O Thread) */
    pthread_create(&cecp_tid, &atrib_threads, cecp_loop, NULL);
    /* Start Search Thread (Worker Thread) */
    pthread_create(&search_tid, &atrib_threads, search_loop, NULL);
    /* Wait for the threads to finish */
    pthread_join(cecp_tid, NULL);
    pthread_join(search_tid, NULL);

    return 0;
}

/* Checks for command-line options */
void cmd_line_options(int argc, char *argv[]) {
    struct option opts[] = {{"level",1,0,'l'},{0,0,0,0}};
    int opt;

    while((opt = getopt_long(argc, argv, "l:", opts, NULL)) != -1) {
        switch(opt) {
        /* The option -l (or --level) selects a difficulty level */
        case 'l':
            level_name = strdup(optarg);
            break;
        }
    }
}

/* Quit function. Aborts program while prints an error message */
void quit(char *s) {
    fprintf(stderr, s);
    exit(1);
}
