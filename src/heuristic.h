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

heuristic.h
Heuristic module header file. Contains all of the weights of the heuristic
components, and also the prototypes of the components.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _HEURISTIC_H_
#define _HEURISTIC_H_

#include "main.h"
#include "board.h"

/* Function prototypes */
int32_t heuristic(board_t *, uint8_t);
int32_t material(board_t *b, uint8_t onmove);
int32_t development(board_t *b, uint8_t onmove);
int32_t pawn(board_t *b, uint8_t onmove);
int32_t passed_pawn(board_t *b, uint8_t onmove);
int32_t isolated_pawn(board_t *b, uint8_t onmove);
int32_t backward_pawn(board_t *b, uint8_t onmove);
int32_t doubled_pawn(board_t *b, uint8_t onmove);
int32_t bishop(board_t *b, uint8_t onmove);
int32_t king(board_t *b, uint8_t onmove);
int32_t knight(board_t *b, uint8_t onmove);
int32_t queen(board_t *b, uint8_t onmove);
int32_t rook(board_t *b, uint8_t onmove);
int32_t control(board_t *b, uint8_t onmove, uint8_t piece, int8_t src_y, int8_t src_x);
void precompute_distances(void);
#endif
