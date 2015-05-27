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

transposition.h
Transposition Table module header file. Contains the definitions of the
transposition cells and tranposition tables, and also some constants used
within those structures.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _TRANSPOSITION_H
#define _TRANSPOSITION_H_

#include "main.h"
#include "board.h"
#include "moves.h"

/* Type of evaluation data stored in transposition cells */
#define TYPE_INVALID 0
#define TYPE_ALPHA 1
#define TYPE_BETA 2
#define TYPE_EXACT 3

/* Transposition table height (maximum number of entries) */
#define TABLE_HEIGHT 4*1024*1024

/* The cell of a transposition table */
typedef struct transposition {
    bitboard_t hash;       /* Position hash key */
    uint8_t type  : 2;     /* Type of data stored (alpha, beta or exact eval) */
    uint8_t depth : 6;     /* Depth of evaluation */
    move_t best;           /* Best move (including evaluation value) */
} transposition_t;   /* 15 bytes */

/* The Transposition table itself */
/* It's just a simple hash table indexed by bitboard hashes */
typedef struct {
    transposition_t *transp;	/* Transposition dynamic-array */
    uint32_t height;		/* Current table height */
    uint32_t entries;		/* Number of transpositions stored */
    uint32_t hits;		/* Number of successful queries */
    uint32_t misses;		/* Number of unsuccessful queries */
} table_t;

void init_transposition_table(void);
void clear_transposition_table(void);
void add_transposition(bitboard_t hash, uint8_t type, uint8_t depth, move_t best);
uint8_t query_transposition(bitboard_t hash, int32_t alpha, int32_t beta, uint8_t depth, move_t *m);
/*
void print_table_stats(void);
void print_table(void);
void print_transposition(transposition_t *t);
*/
#endif
