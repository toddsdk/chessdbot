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

history.h
History of the current chess match (header file). Contains the history structure
definition, some constants and function prototypes.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _HISTORY_H_
#define _HISTORY_H_

#include "main.h"
#include "board.h"
#include "moves.h"

/* History structure */
typedef struct {
    board_t *board; /* History list of boards */
    move_t *move; /* History list of moves*/
    uint32_t size; /* Size (length) of the lists */
    uint32_t max_size; /* Maximum size of both lists */
} history_t;

/* Minimum size of the history lists, and also the size of their increment */
#define HISTORY_PAGE_SIZE 16

/* Function prototypes */
void init_history(void);
void clear_history(void);
void push_history(board_t *b, move_t m);
void pop_history(board_t *b);
board_t *peek_history_board(uint32_t pos);
move_t *peek_history_move(uint32_t pos);
move_t *peek_history_move_top(void);

#endif
