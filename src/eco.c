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

eco.c
Encyclopaedia of Chess Openings (ECO) module. Contains functions and structures
to load, keep and query a tree of possible chess openings that can be played
by the chess engine.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "eco.h"
#include "moves.h"
#include "search.h"
#include "history.h"
#include "xml.h"

bool can_use_eco = TRUE;
static eco_t *eco = NULL;

/* Loads the 'Encyclopaedia of Chess Openings' into the 'eco_t' structure */
bool load_eco(board_t *b) {
    char *san;
    move_t move_coord;
    uint8_t moves, i;
    eco_t *e;
    xml_node_t *root, **opening_list, **opening, **move_list, **mov;

    /* If board is invalid, return error */
    if(b == NULL)
        return FALSE;

    /* Open ECO file */
    root = xml_parser(ECO_FILE);

    eco = init_eco();

    opening_list = get_elements_by_tag_name(root, "opening");
    if(opening_list == NULL)
        quit("Error: Missing 'opening' tag in ECO file!\n");
    /* For each opening */
    for(opening = opening_list; *opening; opening++) {
        moves = 0;
        e = eco;
        move_list = get_elements_by_tag_name(*opening, "move");
        if(move_list == NULL)
            quit("Error: Missing 'opening' tag in ECO file!\n");
        /* For each move of the opening */
        for(mov = move_list; *mov; mov++) {
            san = get_attribute(*mov, "san");
            if(san == NULL)
                quit("Error: Missing 'san' attribute at 'move' element in ECO file!\n");
            /* Translate SAN move to move structure */
            if(!san_to_move(b, san, &move_coord))
                quit("Error: Could not translate from SAN to Coord notation!\n");

            /* Insert the move in the ECO tree */
            e = add_opening_move(e, move_coord);

            /* Follow that move */
            move(b, move_coord);
            moves++;
        }
        /* Set blank move and add it as final token of the opening */
        SET_BLANK_MOVE(move_coord);
        e = add_opening_move(e, move_coord);

        /* Undo moves */
        for(i = 0; i < moves; i++)
            unmove(b);

        free(move_list);
    }
    free(opening_list);

    clean_xml_node(root);

    return TRUE;
}

/* Starts a fresh new, empty eco_t structure */
eco_t *init_eco(void) {
    eco_t *e;
    e = (eco_t *) malloc(sizeof(eco_t));
    if(e == NULL)
        quit("Error: Could not initialize ECO tree!\n");
    memset(e, 0, sizeof(eco_t));
    return e;
}

/* Clean up the entire ECO structure */
void clear_eco(void) {
    clear_eco_node(eco);
    if(eco);
        free(eco);
}

/* Clean up an eco_t node recursively */
void clear_eco_node(eco_t *e) {
    uint32_t i;
    if(e) {
        for(i = 0; i < e->children; i++)
        clear_eco_node(&e->child[i]);
        free(e->child);
    }
}

/* Adds a new opening movement to the ECO tree */
eco_t *add_opening_move(eco_t *e, move_t m) {
    uint8_t i;
    /* Valid opening node? */
    if(e) {
        /* If there is no child yet */
        if(e->child == NULL) {
            e->child = (eco_t *) malloc(sizeof(eco_t));
        } else {
            /* Look for any equal move */
            for(i = 0; i < e->children; i++)
                if(e->child[i].move.src_y == m.src_y && e->child[i].move.src_x == m.src_x &&
                   e->child[i].move.dst_y == m.dst_y && e->child[i].move.dst_x == m.dst_x &&
                   e->child[i].move.promotion == m.promotion)
                    break;
            /* If the move is already there, return a pointer to it */
            if(i < e->children)
                return &e->child[i];
            /* Otherwise, enlarge the child list */
            else
                e->child = (eco_t *) realloc(e->child, (e->children + 1) * sizeof(eco_t));
        }
        if(e->child == NULL)
            quit("Error: Could not add opening move!\n");
        memset(&e->child[e->children], 0, sizeof(eco_t));
        e->child[e->children].move = m;

        e->children++;
        return &e->child[e->children-1];
    } else {
        quit("Error: Could not add opening move!\n");
        return NULL;
    }
}

/* Queries the ECO tree, returning the next move from an opening if the board
 * state allows. */
bool query_eco(move_t *query) {
    uint32_t i, j;
    move_t *m;
    eco_t *e = eco;
    bool match = FALSE;

    if(!can_use_eco)
        return FALSE;

    for(i = 0; (m = peek_history_move(i)) != NULL && e->children > 0; i++) {
        for(j = 0; j < e->children; j++)
            if(m->src_x == e->child[j].move.src_x &&
               m->src_y == e->child[j].move.src_y &&
               m->dst_x == e->child[j].move.dst_x &&
               m->dst_y == e->child[j].move.dst_y &&
               m->promotion == e->child[j].move.promotion)
                break;
        if(j < e->children) {
            e = &e->child[j];
            match = TRUE;
        } else {
            match = FALSE;
            break;
        }
    }

    /* The match has walked towards an opening, or we are the first to move */
    if(match || i == 0) {
        memcpy(query, &e->child[rand() % e->children].move, sizeof(move_t));
        if(query->src_x == 0 && query->src_y == 0 && query->dst_x == 0 &&
           query->dst_y == 0 && query->promotion == NO_PROMOTION)
            return FALSE;
        else
            return TRUE;
    }

    return FALSE;
}

/* Print an eco node. Useful only for debbuging purposes. */
/*
void print_eco(eco_t *e) {
    uint32_t i;
    char coord[8];
    move_to_coord(coord, &e->move);
    printf("ECO: %s\n", coord);
    for(i = 0; i < e->children; i++)
        print_eco(&e->child[i]);
}
*/
