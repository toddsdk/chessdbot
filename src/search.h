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

search.h
Search Thread module header file. Contains some constants to control the search
process and its time constraints, as well as some shared structures (with
CECP Thread) and function prototypes, used out of this module.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "main.h"
#include "board.h"
#include "moves.h"

#define MAX_HEU 999999

#define ALARM_INIT_SEC 3
#define ALARM_INIT_USEC 0

/* Search status */
typedef enum {NOP, FORCE, SEARCH, PONDER, QUIT} status_t;
/* The main (current) board */
extern board_t *board;

extern uint8_t max_depth;

/* Function prototypes */
void *search_loop(void *arg);
move_t alpha_beta(board_t *b, int32_t alpha, int32_t beta, uint32_t ply);
move_t quiescence(board_t *b, int32_t alpha, int32_t beta);
void set_status(status_t s);
status_t get_status(void);
void set_status_and_wait(status_t s);
bool get_timeout(void);
void set_timeout(bool t);
void config_alarm(uint32_t secs);
uint32_t get_config_alarm(void);
void start_alarm(void);
struct timeval stop_alarm(void);
void sigalrm_handler(int useless);
#endif
