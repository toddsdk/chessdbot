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

history.c
History of the current chess match. Contains functions that manipulate the
history of the game in terms of boards and movements that have been made.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "history.h"

static history_t history;

/* (re)Initialize the history of boards and moves */
void init_history(void) {
    clear_history();
    history.size = 0;
    history.max_size = HISTORY_PAGE_SIZE;
    history.board = (board_t *) malloc(history.max_size * sizeof(board_t));
    if(history.board == NULL)
        quit("Error: Could not create history log!\n");
    history.move = (move_t *) malloc(history.max_size * sizeof(move_t));
    if(history.move == NULL)
        quit("Error: Could not create history log!\n");
}

/* Clear the history */
void clear_history(void) {
    if(history.board)
        free(history.board);
    history.board = NULL;
    if(history.move)
        free(history.move);
    history.move = NULL;
}

/* Save a board and a move into the history stack */
void push_history(board_t *b, move_t m) {
    if(history.board) {
        if(history.size + 1 > history.max_size) {
            history.max_size += HISTORY_PAGE_SIZE;
            history.board = (board_t *) realloc(history.board, history.max_size * sizeof(board_t));
            history.move = (move_t *) realloc(history.move, history.max_size * sizeof(move_t));
        }
        if(history.board == NULL || history.move == NULL)
            quit("Error: Could not create history entry!\n");

        history.board[history.size] = *b;
        history.move[history.size] = m;
        history.size++;
    }
}

/* Removes and returns the last added board of the history */
void pop_history(board_t *b) {
    if(history.board && history.size > 0) {
        history.size--;
        memcpy(b, &history.board[history.size], sizeof(board_t));
    }
}

/* Returns the board of an arbitrary position of the history stack */
board_t *peek_history_board(uint32_t pos) {
    if(history.board && history.size > pos)
        return &history.board[pos];
    else
        return NULL;
}

/* Returns the move of an arbitrary position of the history stack */
move_t *peek_history_move(uint32_t pos) {
    if(history.move && history.size > pos)
        return &history.move[pos];
    else
        return NULL;
}

/* Returns the move of the top position of the history stack */
move_t *peek_history_move_top(void) {
    if(history.move && history.size > 0)
        return &history.move[history.size-1];
    else
        return NULL;
}
