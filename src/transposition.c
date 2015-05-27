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

transposition.c
Transposition Table module. Contains functions to manipulate the 'table_t' and
'transposition_t' structures. They, together, are reponsible for saving and
querying the Transpositioned boards (actulally, only the board's evaluations).

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "transposition.h"

/* Transposition Table structure, restricted to this module */
static table_t table;

/* Initialize the transposition table */
void init_transposition_table(void) {
    /* Initial values of table's stats */
    table.entries = 0;
    table.height = TABLE_HEIGHT;
    table.hits = 0;
    table.misses = 0;

    /* Ask for memory for the table entries (the transpositions) */
    table.transp = (transposition_t *) malloc(table.height * sizeof(transposition_t));
    if(table.transp == NULL)
    	quit("Error: Could not create transposition table!\n");

    /* Initialize the entries as TYPE_INVALID, and the other fields as zeros */
    memset(table.transp, 0, table.height * sizeof(transposition_t));
}

/* Clean up memory of transposition table */
void clear_transposition_table(void) {
    if(table.transp)
	free(table.transp);
}

/* Add new element into transposition table */
void add_transposition(bitboard_t hash, uint8_t type, uint8_t depth, move_t best) {
    transposition_t *value;

    /* Reference to the tranposition cell, accordingly to the the hash key */
    value = &table.transp[hash % table.height];

    /* If it's a never-used transposition cell, increase entry count */
    if(value->type == TYPE_INVALID)
    	table.entries++;

    /* Insert new element data */
    value->hash = hash;
    value->type = type;
    value->depth = depth;
    value->best = best;
}

/* Query element from transposition table */
uint8_t query_transposition(bitboard_t hash, int32_t alpha, int32_t beta, uint8_t depth, move_t *best) {
    transposition_t *value = &table.transp[hash % table.height];

    /* Is best moves list empty? */
    if(best == NULL)
    	return TYPE_INVALID;

    /* If it's a hit and the search is deep enough */
    if(value && value->hash == hash && value->depth >= depth) {
	/* Get the best move (evaluation included) */
	*best = value->best;

	/* Check if the new eval is better than the stored one */
    	switch(value->type) {
	case TYPE_ALPHA:
	    if(best->eval >= alpha)
		best->eval = alpha;
	    break;
	case TYPE_BETA:
	    if(best->eval <= beta)
		best->eval = beta;
	    break;
	case TYPE_EXACT:
	case TYPE_INVALID:
	    break;
	}
	table.hits++;
    	return value->type;
    }
    table.misses++;
    return TYPE_INVALID;
}

/* Print the statistics of the Transposition Table -- debug only */
/*
void print_table_stats(void) {
    printf("Transposition Table: [Entries: %u/%u] [Hits/Misses: %u/%u]\n", table.entries, table.height, table.hits, table.misses);
}
*/
/* Print the entire transposition table -- useful only for debugging */
/*
void print_table(void) {
    int i;
    for(i = 0; i < table.height; i++)
    	print_transposition(&table.transp[i]);
}
*/
/* Print a transposition entry of the transp table */
/*
void print_transposition(transposition_t *t) {
    char coord[32] = ""; 
    printf("%016llX ", (uint64_t)t->hash);
    switch(t->type) {
    case TYPE_ALPHA:
    	printf("ALPHA ");
	break;
    case TYPE_BETA:
    	printf("BETA ");
	break;
    case TYPE_EXACT:
    	printf("EXACT ");
	break;
    }
    printf("%d ", t->depth);
    move_to_coord(coord, &t->best);
    printf("%s\n", coord);
}
*/
