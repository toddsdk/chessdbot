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

levels.h
Level selection module header file. Contains the definition of a structure that
represents the settings of a particular difficulty level, such as maximum search
depth, heuristic weights, etc.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _LEVELS_H_
#define _LEVES_H_

#include "main.h"
#include "xml.h"

#define LEVELS_CONFIG_FILE BASE_DIR "levels.xml"

typedef struct {
    char *name;
    int max_depth;
    int max_seconds;
    int pawn_val;
    int bishop_val;
    int knight_val;
    int rook_val;
    int queen_val;
    int king_val;
    int factor_material;
    int factor_development;
    int factor_pawn;
    int factor_bishop;
    int factor_king;
    int factor_knight;
    int factor_queen;
    int factor_rook;
    int bonus_early_queen_move;
    int bonus_early_bishop_stuck;
    int bonus_early_knight_stuck;
    int bonus_has_castled;
    int bonus_hasnt_castled;
    int bonus_passed_pawn;
    int bonus_isolated_pawn;
    int bonus_backward_pawn;
    int bonus_doubled_pawn;
    int bonus_tripled_pawn;
    int bonus_doubled_bishop;
    int bonus_fianchetto_bishop;
    int bonus_knight_on_edge;
    int bonus_knight_on_hole;
    int bonus_rook_open_file;
    int bonus_rook_halfopen_file;
    int bonus_queen_open_file;
    int bonus_queen_halfopen_file;
    int bonus_center_control;
} level_t;

/* The Global selected level configuration. */
extern level_t *config;

void validate_level(char *lvl);
void adjust_level(char *lvl);
void load_levels();
void select_level(char *lvl);

#endif
