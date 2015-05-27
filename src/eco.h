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

eco.h
Encyclopaedia of Chess Openings (ECO) module header file. Contains ECO Tree
definition, ECO manipulation function prototypes, and also a path for the
'eco.dat' file, that contains all the openings.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _ECO_H_
#define _ECO_H_

#include "main.h"
#include "board.h"
#include "moves.h"

extern bool can_use_eco;

/* Path to the file that contains the chess openings */
#define ECO_FILE BASE_DIR "eco.xml"

/* ECO Tree structure */
typedef struct eco eco_t;
struct eco {
    move_t move;      /* Opening move */
    uint8_t children; /* Number of children nodes */
    eco_t *child;     /* List of children nodes */
};

/* Function prototypes */
bool load_eco(board_t *b);
eco_t *init_eco(void);
void clear_eco(void);
void clear_eco_node(eco_t *e);
eco_t *add_opening_move(eco_t *e, move_t m);
bool query_eco(move_t *query);
void print_eco(eco_t *e);

#endif
