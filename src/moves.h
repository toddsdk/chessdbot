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

moves.h
Movements module header file. Contains definitions of structures, macros and
function prototypes.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _MOVES_H_
#define _MOVES_H_

#include "main.h"
#include "board.h"

/* Movement structure */
typedef struct {
    uint16_t src_y : 3; /* Source rank */
    uint16_t src_x : 3; /* Source file */
    uint16_t dst_y : 3; /* Destination rank */
    uint16_t dst_x : 3; /* Destination file */
    uint16_t promotion : 3; /* No promotion + 4 pieces = 5 states */
    uint16_t fill : 1; /* unnused, just to fit in 16 bits */
    int32_t eval; /* Movement (heuristic) evaluation */
} move_t; /* 6 bytes */

/* Movements manipulation macros */
#define SET_BLANK_MOVE(m) memset(&m, 0, sizeof(move_t))
#define IS_BLANK_MOVE(m) (((m).dst_x == 0) && ((m).dst_y == 0) && ((m).src_x == 0) && ((m).src_y == 0) && ((m).promotion == 0) && ((m).eval == 0))

/* Minimum size of a move list and also the increase size when a list is full */
#define MOVE_LIST_PAGE_SIZE 32

/* Move List structure */
typedef struct {
    move_t *move; /* List of movements */
    uint32_t size; /* Size of the list */
    uint32_t max_size; /* Maximum size of the list */
} move_list_t;

/* Move tables for knights, kings and sliding pieces (queen, rook and bishop) */
extern bitboard_t moves_knight[8][8], moves_king[8][8];
extern bitline_t moves_slide[8][256];

/* Function prototypes */
move_list_t *init_move_list(void);
void add_move(move_list_t *l, move_t m);
void clear_move_list(move_list_t *list);
move_list_t *gen_move_list(board_t *b, bool captures_only);
void gen_pawn(board_t *b, move_list_t *list, bool captures_only);
void gen_knight(board_t *b, move_list_t *list, bool captures_only);
void gen_bishop(board_t *b, move_list_t *list, bool captures_only);
void gen_rook(board_t *b, move_list_t *list, bool captures_only);
void gen_queen(board_t *b, move_list_t *list, bool captures_only);
void gen_king(board_t *b, move_list_t *list, bool captures_only);
move_t gen_move(uint8_t src_y, uint8_t src_x, uint8_t dst_y, uint8_t dst_x, uint8_t promo);
void move(board_t *b, move_t m);
void unmove(board_t *b);
bool coord_to_move(char *c, move_t *m);
bool move_to_coord(char *c, move_t *m);
bool san_to_move(board_t *b, char *s, move_t *m);
void precompute_moves(void);
bool check_valid_move(board_t *b, move_t m);
#endif
