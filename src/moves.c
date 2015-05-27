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

moves.c
Movements module. Contains the functions that generate all the possible moves
(in a structure called 'move_list_t') starting from a specific state (board).

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "moves.h"
#include "history.h"

/* Boards used in knight's and king's move generation */
bitboard_t moves_knight[8][8], moves_king[8][8];
/* Rows used in rook's, bishop's and queen's move generation */
bitline_t moves_slide[8][256];

/* Creates a move list.
 * Return a new empty move list. NULL is returned if an error occur */
move_list_t *init_move_list(void) {
    move_list_t *l;

    /* Alloc memory to the move list, fill it with zeros */
    l = (move_list_t *) malloc(sizeof(move_list_t));
    if(l == NULL)
        return NULL;
    l = memset(l, 0, sizeof(move_list_t));

    /* Set move list default settings*/
    l->size = 0;
    l->max_size = MOVE_LIST_PAGE_SIZE;


    /* Alloc memory to the moves and fill them with zeros */
    l->move = (move_t *) malloc(l->max_size * sizeof(move_t));
    if(l->move == NULL) {
        free(l);
        return NULL;
    }
    l->move = memset(l->move, 0, l->max_size * sizeof(move_t));
    return l;
}

/* Adds a new move to a previously allocated move list */
void add_move(move_list_t *l, move_t m) {
    /* If the move list is valid */
    if(l) {
        /* If we reached the max size, ask for more space */
        if(l->size + 1 > l->max_size) {
            l->max_size += MOVE_LIST_PAGE_SIZE;
            l->move = (move_t *) realloc(l->move, l->max_size * sizeof(move_t));
            if(l->move == NULL)
                quit("Error: Could not add move to move list!\n");
        }

        /* Properly adds the move to the list */
        l->move[l->size] = m;

        /* Updates move list's current size */
        l->size++;
    }
}

/* Clears a move list, freeing its memory */
void clear_move_list(move_list_t *list) {
    if(list) {
        if(list->move)
            free(list->move);
        free(list);
    }
}

/* Generate a legal move list */
move_list_t *gen_move_list(board_t *b, bool captures_only) {
    uint8_t i, onmove = b->onmove;
    move_list_t *illegal, *legal;

    /* Initialize the list of all possible moves (including illegal ones) */
    illegal = init_move_list();

    /* Generate all the moves, for each kind of piece */
    gen_pawn(b, illegal, captures_only);
    gen_bishop(b, illegal, captures_only);
    gen_knight(b, illegal, captures_only);
    gen_rook(b, illegal, captures_only);
    gen_queen(b, illegal, captures_only);
    gen_king(b, illegal, captures_only);

    /* Start a list of all the legal moves */
    legal = init_move_list();
    /* For each move on the illegal list, check if it is legal */
    for(i = 0; i < illegal->size; i++) {
        move(b, illegal->move[i]);
        /* If the move doesn't leave us in check, add it to the legal list */
        if(!check(b, b->bitboard[onmove][KING], !onmove))
            add_move(legal, illegal->move[i]);
        unmove(b);
    }

    /* Clear the illegal move list */
    clear_move_list(illegal);

    return legal;
}

/* Generate pawn's moves and add them to a move list */
void gen_pawn(board_t *b, move_list_t *list, bool captures_only) {
    int8_t src, src_y, src_x, dst, dst_y, dst_x, onmove = b->onmove, i, j;
    bitboard_t from = b->bitboard[onmove][PAWN], to;

    /* Capture moves */
    for(to = from; (src = FIRST_BIT(to)) != -1; CLEAR_BIT(to, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        for(i = (onmove ? 1 : -1), j = -1; j <= 1; j += 2) {
            dst_y = src_y + i;
            dst_x = src_x + j;
            if(dst_y <= RANK_8 && dst_y >= RANK_1 && dst_x <= FILE_A && dst_x >= FILE_H)
                if(GET_BIT(b->rotation[!onmove][ROT_0], dst_y, dst_x)) {
                    if(onmove ? dst_y == RANK_8 : dst_y == RANK_1) {
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, QUEEN));
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, ROOK));
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, KNIGHT));
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, BISHOP));
                    } else {
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
                    }
                }
        }
    }

    /* Enpassant moves */
    if(ENPASSANT_GET_VALID(b->enpassant)) {
        dst_y = onmove ? RANK_6 : RANK_3;
           src_y = onmove ? RANK_5 : RANK_4;
        dst_x = ENPASSANT_GET_FILE(b->enpassant);
        src_x = dst_x - 1;
        if(src_x >= FILE_H && GET_BIT(from, src_y, src_x))
            add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
        src_x = dst_x + 1;
        if(src_x <= FILE_A && GET_BIT(from, src_y, src_x))
            add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
    }

    /* If we are generating only the capture moves, get out */
    if(captures_only)
        return;

    /* Single moves */
    to = onmove ? ((from << 8) & ~(b->rotation[COLORS][ROT_0])) : ((from >> 8) & ~(b->rotation[COLORS][ROT_0]));
    for(; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
        dst_y = dst/8;
        dst_x = dst%8;
        if(onmove) {
            if(dst_y == RANK_8) {
                add_move(list, gen_move(RANK_7, dst_x, dst_y, dst_x, QUEEN));
                add_move(list, gen_move(RANK_7, dst_x, dst_y, dst_x, ROOK));
                add_move(list, gen_move(RANK_7, dst_x, dst_y, dst_x, KNIGHT));
                add_move(list, gen_move(RANK_7, dst_x, dst_y, dst_x, BISHOP));
            } else {
                add_move(list, gen_move(dst_y - 1, dst_x, dst_y, dst_x, NO_PROMOTION));
            }
        } else {
            if(dst_y == RANK_1) {
                add_move(list, gen_move(RANK_2, dst_x, dst_y, dst_x, QUEEN));
                add_move(list, gen_move(RANK_2, dst_x, dst_y, dst_x, ROOK));
                add_move(list, gen_move(RANK_2, dst_x, dst_y, dst_x, KNIGHT));
                add_move(list, gen_move(RANK_2, dst_x, dst_y, dst_x, BISHOP));
            } else {
                add_move(list, gen_move(dst_y + 1, dst_x, dst_y, dst_x, NO_PROMOTION));
            }
        }
    }

    /* Double moves */
    if(onmove) {
        to = ((from & rank[RANK_2]) << 8) & ~(b->rotation[COLORS][ROT_0]);
        to = (to << 8) & ~(b->rotation[COLORS][ROT_0]);
    } else {
        to = ((from & rank[RANK_7]) >> 8) & ~(b->rotation[COLORS][ROT_0]);
        to = (to >> 8) & ~(b->rotation[COLORS][ROT_0]);
    }

    for(; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
        dst_y = dst/8;
        dst_x = dst%8;
        add_move(list, gen_move((onmove ? RANK_2 : RANK_7), dst_x, dst_y, dst_x, NO_PROMOTION));
    }

}

/* Generate knight's moves and add them to a move list */
void gen_knight(board_t *b, move_list_t *list, bool captures_only) {
    int8_t src, src_y, src_x, dst, dst_y, dst_x, onmove = b->onmove;
    bitboard_t from = b->bitboard[onmove][KNIGHT], to;

    for(; (src = FIRST_BIT(from)) != -1; CLEAR_BIT(from, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        to = moves_knight[src_y][src_x] & ~(b->rotation[onmove][ROT_0]);
        for(; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
            dst_y = dst/8;
            dst_x = dst%8;
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], dst_y, dst_x))
                add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
        }
    }
}

/* Generate bishop's moves and add them to a move list */
void gen_bishop(board_t *b, move_list_t *list, bool captures_only) {
    int8_t src, src_y, src_x, dst, dst_x, rot_y, rot_x, unrot_y, unrot_x, onmove = b->onmove;
    bitboard_t from = b->bitboard[onmove][BISHOP];
    bitline_t line, to;

    for(; (src = FIRST_BIT(from)) != -1; CLEAR_BIT(from, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        rot_y = rot_map[ROT_45][src_y][src_x][Y];
        rot_x = rot_map[ROT_45][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_45], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_45[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_45], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_45][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_45][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
        rot_y = rot_map[ROT_315][src_y][src_x][Y];
        rot_x = rot_map[ROT_315][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_315], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_315[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_315], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_315][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_315][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
    }
}

/* Generate rook's moves and add them to a move list */
void gen_rook(board_t *b, move_list_t *list, bool captures_only) {
    int8_t src, src_y, src_x, dst, dst_x, rot_y, rot_x, unrot_y, unrot_x, onmove = b->onmove;
    bitboard_t from = b->bitboard[onmove][ROOK];
    bitline_t line, to;

    for(; (src = FIRST_BIT(from)) != -1; CLEAR_BIT(from, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        rot_y = rot_map[ROT_0][src_y][src_x][Y];
        rot_x = rot_map[ROT_0][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_0], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_0], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_0][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_0][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
        rot_y = rot_map[ROT_90][src_y][src_x][Y];
        rot_x = rot_map[ROT_90][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_90], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_90], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_90][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_90][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
    }
}

/* Generate queen's moves and add them to a move list */
void gen_queen(board_t *b, move_list_t *list, bool captures_only) {
    int8_t src, src_y, src_x, dst, dst_x, rot_y, rot_x, unrot_y, unrot_x, onmove = b->onmove;
    bitboard_t from = b->bitboard[onmove][QUEEN];
    bitline_t line, to;

    for(; (src = FIRST_BIT(from)) != -1; CLEAR_BIT(from, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        rot_y = rot_map[ROT_0][src_y][src_x][Y];
        rot_x = rot_map[ROT_0][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_0], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_0], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_0][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_0][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
        rot_y = rot_map[ROT_90][src_y][src_x][Y];
        rot_x = rot_map[ROT_90][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_90], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_90], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_90][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_90][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
        rot_y = rot_map[ROT_45][src_y][src_x][Y];
        rot_x = rot_map[ROT_45][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_45], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_45[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_45], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_45][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_45][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
        rot_y = rot_map[ROT_315][src_y][src_x][Y];
        rot_x = rot_map[ROT_315][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_315], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_315[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_315], rot_y);
        for(; (dst = FIRST_BIT_LINE(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_315][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_315][rot_y][dst_x][X];
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], unrot_y, unrot_x))
                add_move(list, gen_move(src_y, src_x, unrot_y, unrot_x, NO_PROMOTION));
        }
    }
}

/* Generate king's moves and add them to a move list */
void gen_king(board_t *b, move_list_t *list, bool captures_only) {
    /* Squares used in king's move generation */
    static bitboard_t check_squares[COLORS][CASTLE_SIDES]  = {{0x3800000000000000ULL,0x0E00000000000000ULL},{0x0000000000000038ULL,0x000000000000000EULL}},
              free_squares[COLORS][CASTLE_SIDES] = {{0x7000000000000000ULL,0x0600000000000000ULL},{0x0000000000000070ULL,0x0000000000000006ULL}};
    int8_t src, src_y, src_x, dst, dst_y, dst_x, onmove = b->onmove, side;
    bitboard_t from = b->bitboard[onmove][KING], to;

    /* Castle moves */
    for(side = QSIDE; side < CASTLE_SIDES; side++)
        if(CAN_CASTLE(b->castle, side, onmove))
            if(!(free_squares[onmove][side] & b->rotation[COLORS][ROT_0]))
                if(!check(b, check_squares[onmove][side], !onmove)) {
                    src_x = FILE_E;
                    src_y = dst_y = onmove ? RANK_1 : RANK_8;
                    dst_x = (side == QSIDE ? src_x+2 : src_x-2);
                    if(!captures_only)
                        add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
                }

    /* Normal king moves (captures also) */
    for(; (src = FIRST_BIT(from)) != -1; CLEAR_BIT(from, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;
        to = moves_king[src_y][src_x] & ~(b->rotation[onmove][ROT_0]);
        for(; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
            dst_y = dst/8;
            dst_x = dst%8;
            if(!captures_only || GET_BIT(b->rotation[!onmove][ROT_0], dst_y, dst_x))
                add_move(list, gen_move(src_y, src_x, dst_y, dst_x, NO_PROMOTION));
        }
    }
}


/* Return a well-formed move structure, with the given parameters into it */
move_t gen_move(uint8_t src_y, uint8_t src_x, uint8_t dst_y, uint8_t dst_x, uint8_t promo) {
    move_t m;
    memset(&m, 0, sizeof(move_t));
    m.src_y = src_y;
    m.src_x = src_x;
    m.dst_y = dst_y;
    m.dst_x = dst_x;
    m.promotion = promo;
    return m;
}

/* Reorders a move list, letting the best captures first */
void reorder_move_list(board_t *b, move_list_t *list) {
    static uint8_t piece_value[] = {1, 3, 5, 5, 9, 100};
    int8_t piece, attacker = -1, victim = -1, i, j, max;
    move_t swap;

    /* Set the moves' evaluation, following the MVV/LVA rule
     * (Most Valuable Victim/Least Valuable Attacker) */
    for(i = 0; i < list->size; i++) {
        for(piece = PAWN; piece < PIECES; piece++) {
            if(GET_BIT(b->bitboard[piece][b->onmove], list->move[i].src_y, list->move[i].src_x))
                attacker = piece;
            if(GET_BIT(b->bitboard[piece][b->onmove], list->move[i].dst_y, list->move[i].dst_x))
                victim = piece;
        }
        /* If it's really a capture move, set the evaluation */
        if(attacker != -1 && victim != -1)
            list->move[i].eval = piece_value[attacker] / piece_value[victim];
        else
            return;
    }

    /* Order the move list */
    for(i = 0; i < list->size; i++) {
        max = i;
        for(j = i+1; j < list->size; j++)
            if(list->move[j].eval > list->move[max].eval)
                max = j;
        swap = list->move[i];
        list->move[i] = list->move[max];
        list->move[max] = swap;
    }
}

/* Perform a given move on a given board, even if it's an invalid move */
void move(board_t *b, move_t m) {
    uint8_t piece, onmove = b->onmove, rot;
    bool capture = FALSE;

    /* Save current state */
    push_history(b, m);

    for(piece = PAWN; piece < PIECES; piece++) {
        /* Check the type of piece that has moved */
        if(GET_BIT(b->bitboard[onmove][piece], m.src_y, m.src_x)) {

            /* Clear old position and set the new one */
            CLEAR_BIT(b->bitboard[onmove][piece], m.src_y, m.src_x);
            SET_BIT(b->bitboard[onmove][piece], m.dst_y, m.dst_x);

            /* Do it on the rotated bitboards */
            for(rot = ROT_0; rot < ROTATIONS; rot++) {
                CLEAR_BIT(b->rotation[onmove][rot], rot_map[rot][m.src_y][m.src_x][Y], rot_map[rot][m.src_y][m.src_x][X]);
                SET_BIT(b->rotation[onmove][rot], rot_map[rot][m.dst_y][m.dst_x][Y], rot_map[rot][m.dst_y][m.dst_x][X]);
            }

            /* Set the new hash */
            b->hash ^= zobrist_piece[onmove][piece][m.src_y][m.src_x];
            b->hash ^= zobrist_piece[onmove][piece][m.dst_y][m.dst_x];
        }

        /* If it's a capture, clear the captured piece */
        if(GET_BIT(b->bitboard[!onmove][piece], m.dst_y, m.dst_x)) {
            CLEAR_BIT(b->bitboard[!onmove][piece], m.dst_y, m.dst_x);
            for(rot = ROT_0; rot < ROTATIONS; rot++)
                CLEAR_BIT(b->rotation[!onmove][rot], rot_map[rot][m.dst_y][m.dst_x][Y], rot_map[rot][m.dst_y][m.dst_x][X]);
            b->hash ^= zobrist_piece[!onmove][piece][m.dst_y][m.dst_x];
            capture = TRUE;
        }
    }

    /* If a rook is moving for the first time, set castle rights properly */
    if((m.src_y == (onmove ? RANK_1 : RANK_8)) && (m.src_x == FILE_H || m.src_x == FILE_A) && CAN_CASTLE(b->castle, m.src_x == FILE_H, onmove)) {
        b->hash ^= zobrist_castle[b->castle];
        CLEAR_CASTLE(b->castle, (m.src_x == FILE_H ? KSIDE : QSIDE), onmove);
        b->hash ^= zobrist_castle[b->castle];
    }

    /* If a piece moved to an enemy's rook first square, set castle rights properly */
    if((m.dst_y == (onmove ? RANK_8 : RANK_1)) && (m.dst_x == FILE_H || m.dst_x == FILE_A) && CAN_CASTLE(b->castle, m.dst_x == FILE_H, !onmove)) {
        b->hash ^= zobrist_castle[b->castle];
        CLEAR_CASTLE(b->castle, (m.dst_x == FILE_H ? KSIDE : QSIDE), !onmove);
        b->hash ^= zobrist_castle[b->castle];
    }

    /* The king has been moved? */
    if(GET_BIT(b->bitboard[onmove][KING], m.dst_y, m.dst_x)) {
        /* Move the rook if the move is a castle */
        if(m.src_x == FILE_E && (m.dst_x == FILE_C || m.dst_x == FILE_G)) {
            CLEAR_BIT(b->bitboard[onmove][ROOK], m.dst_y, (m.dst_x == FILE_G ? FILE_H : FILE_A));
            SET_BIT(b->bitboard[onmove][ROOK], m.dst_y, (m.dst_x == FILE_G ? FILE_F : FILE_D));

            for(rot = ROT_0; rot < ROTATIONS; rot++) {
                CLEAR_BIT(b->rotation[onmove][rot], rot_map[rot][m.dst_y][m.dst_x == FILE_G ? FILE_H : FILE_A][Y], rot_map[rot][m.dst_y][m.dst_x == FILE_G ? FILE_H : FILE_A][X]);
                SET_BIT(b->rotation[onmove][rot], rot_map[rot][m.dst_y][m.dst_x == FILE_G ? FILE_F : FILE_D][Y], rot_map[rot][m.dst_y][m.dst_x == FILE_G ? FILE_F : FILE_D][X]);
            }

            b->hash ^= zobrist_piece[onmove][ROOK][m.dst_y][m.dst_x == FILE_G ? FILE_H : FILE_A];
            b->hash ^= zobrist_piece[onmove][ROOK][m.dst_y][m.dst_x == FILE_G ? FILE_F : FILE_D];

            b->castled |= (onmove ? WHITE_CASTLED : BLACK_CASTLED);
        }

        /* The onmove side cannot castle anymore */
        if(CAN_CASTLE(b->castle, CASTLE_SIDES, onmove)) {
            b->hash ^= zobrist_castle[b->castle];
            CLEAR_CASTLE(b->castle, CASTLE_SIDES, onmove);
            b->hash ^= zobrist_castle[b->castle];
        }
    }

    /* A pawn has been moved? */
    if(GET_BIT(b->bitboard[onmove][PAWN], m.dst_y, m.dst_x)) {
        /* Change the piece in case of promotion */
        if(m.promotion != NO_PROMOTION) {
            CLEAR_BIT(b->bitboard[onmove][PAWN], m.dst_y, m.dst_x);
            SET_BIT(b->bitboard[onmove][m.promotion], m.dst_y, m.dst_x);
            b->hash ^= zobrist_piece[onmove][PAWN][m.dst_y][m.dst_x];
            b->hash ^= zobrist_piece[onmove][m.promotion][m.dst_y][m.dst_x];
        }

        /* Capture the pawn if it's enpassant move */
        if(ENPASSANT_GET_VALID(b->enpassant) && m.dst_x == ENPASSANT_GET_FILE(b->enpassant) && m.dst_y == (onmove ? RANK_6 : RANK_3)) {
            CLEAR_BIT(b->bitboard[!onmove][PAWN], (onmove ? RANK_5 : RANK_4), m.dst_x);
            for(rot = ROT_0; rot < ROTATIONS; rot++)
                CLEAR_BIT(b->rotation[!onmove][rot], rot_map[rot][onmove ? RANK_5 : RANK_4][m.dst_x][Y], rot_map[rot][onmove ? RANK_5 : RANK_4][m.dst_x][X]);
            b->hash ^= zobrist_piece[!onmove][PAWN][onmove ? RANK_5 : RANK_4][m.dst_x];
        }

        /* Set enpassant flags if double move */
        b->hash ^= zobrist_enpassant[b->enpassant];
        if(onmove ? m.src_y == RANK_2 && m.dst_y == RANK_4 : m.src_y == RANK_7 && m.dst_y == RANK_5) {
            ENPASSANT_SET_VALID(b->enpassant, TRUE);
            ENPASSANT_SET_FILE(b->enpassant, m.src_x);
        /* Set invalid enpassant if not double move*/
        } else {
            ENPASSANT_SET_VALID(b->enpassant, FALSE);
            ENPASSANT_SET_FILE(b->enpassant, 0);
        }
        b->hash ^= zobrist_enpassant[b->enpassant];

        /* Reset Halfmove counter */
        b->hm = 0;
    } else {
        /* Set invalid enpassant */
        b->hash ^= zobrist_enpassant[b->enpassant];
        ENPASSANT_SET_VALID(b->enpassant, FALSE);
        ENPASSANT_SET_FILE(b->enpassant, 0);
        b->hash ^= zobrist_enpassant[b->enpassant];

        /* Reset Halfmove counter if a capture occured */
        if(capture)
            b->hm = 0;
        /* Increment Halfmove counter neither a capture took place nor a pawn moved */
        else
            b->hm++;
    }

    /* Increment Fullmove counter */
    if(onmove == COLOR_BLACK)
        b->fm++;

    /* Change player onmove */
    b->onmove = !b->onmove;
    b->hash ^= zobrist_white_onmove;

    /* Update rotated bitboards */
    for(rot = ROT_0; rot < ROTATIONS; rot++)
        b->rotation[COLORS][rot] = b->rotation[COLOR_BLACK][rot] | b->rotation[COLOR_WHITE][rot];
}

/* Restore previous state */
void unmove(board_t *b) {
    pop_history(b);
}

/* Translate moves in Coordinate notation to internal move structure format */
bool coord_to_move(char *c, move_t *m) {
    uint8_t len;

    if(c == NULL || m == NULL)
        return FALSE;

    /* Check valid coord length */
    /* i.e. d2d4, f6a1, e1g1, b7b8q, h7h8n */
    len = strlen(c);
    if(len > 5 || len < 4)
        return FALSE;

    /* Check for valid coords */
    if(c[0] >= 'a' && c[0] <= 'h' && c[2] >= 'a' && c[2] <= 'h' &&
       c[1] >= '1' && c[1] <= '8' && c[3] >= '1' && c[3] <= '8') {
        m->src_x = 'h' - c[0];
        m->src_y = c[1] - '1';
        m->dst_x = 'h' - c[2];
        m->dst_y = c[3] - '1';
        if(len == 5) {
            switch(c[4]) {
            case 'q':
                m->promotion = QUEEN;
                break;
            case 'r':
                m->promotion = ROOK;
                break;
            case 'b':
                m->promotion = BISHOP;
                break;
            case 'n':
                m->promotion = KNIGHT;
                break;
            default:
                return FALSE;
            }
        } else {
            m->promotion = NO_PROMOTION;
        }
    }

    return TRUE;
}

/* Translate move structure to Coordinate Notation (string format) */
bool move_to_coord(char *c, move_t *m) {
    if(c == NULL || m == NULL)
        return FALSE;

    c[0] = 'h' - m->src_x;
    c[1] = m->src_y + '1';
    c[2] = 'h' - m->dst_x;
    c[3] = m->dst_y + '1';
    switch(m->promotion) {
    case NO_PROMOTION:
        c[4] = '\0';
        break;
    case KNIGHT:
        c[4] = 'n';
        c[5] = '\0';
        break;
    case BISHOP:
        c[4] = 'b';
        c[5] = '\0';
        break;
    case ROOK:
        c[4] = 'r';
        c[5] = '\0';
        break;
    case QUEEN:
        c[4] = 'q';
        c[5] = '\0';
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

/* Translates short algebraic notation (SAN) to coordinate notation */
bool san_to_move(board_t *b, char *s, move_t *m) {
    int16_t x1, y1, x2, y2, i, src, piece;
    move_list_t *list = NULL;
    bitboard_t bitboard = 0;

    x1 = y1 = x2 = y2 = -1;

    if(b == NULL || s == NULL || m == NULL)
        return FALSE;

    /* In case it's a castle move */
    if(!strncmp(s, "O-O", 3)) {
        m->src_y = m->dst_y = b->onmove == COLOR_WHITE ? 0 : 7;
        m->src_x = 3;
        m->dst_x = 1;
        return TRUE;
    } else if(!strncmp(s, "O-O-O", 5)) {
        m->src_y = m->dst_y = b->onmove == COLOR_WHITE ? 0 : 7;
        m->src_x = 3;
        m->dst_x = 5;
        return TRUE;
    }

    /* Get piece type */
    if(isupper(*s)) {
        switch(toupper(*s)) {
        case 'B':
            piece = BISHOP;
            break;
        case 'K':
            piece = KING;
            break;
        case 'N':
            piece = KNIGHT;
            break;
        case 'Q':
            piece = QUEEN;
            break;
        case 'R':
            piece = ROOK;
            break;
        default:
            return FALSE;
        }
        s++;
    } else {
        piece = PAWN;
    }

    /* Parses source and destination squares */
    if(*s >= 'a' && *s <= 'h') {
        x1 = 'h' - *s;
        s++;
    }
    if(*s >= '1' && *s <= '8') {
        y1 = *s - '1';
        s++;
    }
    if(*s == 'x')
        s++;
    if(*s >= 'a' && *s <= 'h') {
        x2 = 'h' - *s;
        s++;
    }
    if(*s >= '1' && *s <= '8') {
        y2 = *s - '1';
        s++;
    }

    /* If it's a pawn moving and also promoting */
    if(piece == PAWN && (*s == '=' || *s == '(')) {
        s++;
    switch(toupper(*s)) {
    case 'B':
        m->promotion = BISHOP;
        break;
    case 'N':
        m->promotion = KNIGHT;
        break;
    case 'Q':
        m->promotion = QUEEN;
        break;
    case 'R':
        m->promotion = ROOK;
        break;
    default:
        return FALSE;
        }
    } else {
        m->promotion = NO_PROMOTION;
    }

    /* Discover the remaining square's info */
    if(x1 != -1 || y1 != -1) {
        if(x1 != -1 && y1 != -1 && x2 != -1 && y2 != -1) {
            m->src_x = x1;
            m->src_y = y1;
            m->dst_x = x2;
            m->dst_y = y2;
        } else if(x1 != -1 && y1 == -1 && x2 != -1 && y2 != -1) {
            for(i = 0; i < 8; i++)
                SET_BIT(bitboard, i, x1);
            src = FIRST_BIT(bitboard & b->bitboard[b->onmove][piece]);
            m->src_x = x1;
            m->src_y = src/8;
            m->dst_x = x2;
            m->dst_y = y2;
        } else if(x1 == -1 && y1 != -1 && x2 != -1 && y2 != -1) {
            for(i = 0; i < 8; i++)
                SET_BIT(bitboard, y1, i);
            src = FIRST_BIT(bitboard & b->bitboard[b->onmove][piece]);
            m->src_x = src%8;
            m->src_y = y1;
            m->dst_x = x2;
            m->dst_y = y2;
        } else if(x1 != -1 && y1 != -1 && x2 == -1 && y2 == -1) {
            m->dst_x = x1;
            m->dst_y = y1;
            list = init_move_list();
            switch(piece) {
            case BISHOP:
                gen_bishop(b, list, FALSE);
                break;
            case KING:
                gen_king(b, list, FALSE);
                break;
            case KNIGHT:
                gen_knight(b, list, FALSE);
                break;
            case QUEEN:
                gen_queen(b, list, FALSE);
                break;
            case ROOK:
                gen_rook(b, list, FALSE);
                break;
            case PAWN:
                gen_pawn(b, list, FALSE);
                break;
            }
            for(i = 0; i < list->size; i++)
                if(x1 == list->move[i].dst_x && y1 == list->move[i].dst_y)
                    break;
            if(i < list->size) {
                m->src_x = list->move[i].src_x;
                m->src_y = list->move[i].src_y;
            } else {
                return FALSE;
            }
            clear_move_list(list);
        } else {
            return FALSE;
        }
    } else if(x2 != -1 && y2 != -1) {
        m->dst_x = x2;
        m->dst_y = y2;
        list = init_move_list();
        switch(piece) {
        case BISHOP:
            gen_bishop(b, list, FALSE);
            break;
        case KING:
            gen_king(b, list, FALSE);
            break;
        case KNIGHT:
            gen_knight(b, list, FALSE);
            break;
        case QUEEN:
            gen_queen(b, list, FALSE);
            break;
        case ROOK:
            gen_rook(b, list, FALSE);
            break;
        case PAWN:
            gen_pawn(b, list, FALSE);
            break;
        }
        for(i = 0; i < list->size; i++)
            if(x2 == list->move[i].dst_x && y2 == list->move[i].dst_y)
                break;
        if(i < list->size) {
            m->src_x = list->move[i].src_x;
            m->src_y = list->move[i].src_y;
        } else {
            return FALSE;
        }
        clear_move_list(list);
    } else {
        return FALSE;
    }

    return TRUE;
}

/* Generate move tables used extensively by move generation */
void precompute_moves(void) {
    uint8_t x, y;
    int16_t line, i, j;

    /* Knight moves */
    for(y = 0; y < 8; y++)
        for(x = 0; x < 8; x++) {
            moves_knight[y][x] = 0;
            if(y-2 >= 0 && x-1 >= 0)
                SET_BIT(moves_knight[y][x],y-2,x-1);
            if(y-2 >= 0 && x+1 <= 7)
                SET_BIT(moves_knight[y][x],y-2,x+1);
            if(y-1 >= 0 && x-2 >= 0)
                SET_BIT(moves_knight[y][x],y-1,x-2);
            if(y-1 >= 0 && x+2 <= 7)
                SET_BIT(moves_knight[y][x],y-1,x+2);
            if(y+1 <= 7 && x-2 >= 0)
                SET_BIT(moves_knight[y][x],y+1,x-2);
            if(y+1 <= 7 && x+2 <= 7)
                SET_BIT(moves_knight[y][x],y+1,x+2);
            if(y+2 <= 7 && x-1 >= 0)
                SET_BIT(moves_knight[y][x],y+2,x-1);
            if(y+2 <= 7 && x+1 <= 7)
                SET_BIT(moves_knight[y][x],y+2,x+1);
        }

    /* King moves */
    for(y = 0; y < 8; y++)
        for(x = 0; x < 8; x++) {
            moves_king[y][x] = 0;
            if(y-1 >= 0 && x-1 >= 0)
                SET_BIT(moves_king[y][x],y-1,x-1);
            if(y-1 >= 0)
                SET_BIT(moves_king[y][x],y-1,x);
            if(y-1 >= 0 && x+1 <= 7)
                SET_BIT(moves_king[y][x],y-1,x+1);
            if(x-1 >= 0)
                SET_BIT(moves_king[y][x],y,x-1);
            if(x+1 <= 7)
                SET_BIT(moves_king[y][x],y,x+1);
            if(y+1 <= 7 && x-1 >= 0)
                SET_BIT(moves_king[y][x],y+1,x-1);
            if(y+1 <= 7)
                SET_BIT(moves_king[y][x],y+1,x);
            if(y+1 <= 7 && x+1 <= 7)
                SET_BIT(moves_king[y][x],y+1,x+1);
        }

    /* Slide moves (Rook, Bishop & Queen) */
    for(j = 0; j < 8; j++)
        for(line = 0; line < 256; line++) {
            moves_slide[j][line] = 0;
            for(i = 1; j+i < 8; i++) {
                SET_BIT(moves_slide[j][line],0,j+i);
                if(GET_BIT(line,0,j+i))
                    break;
        }
            for(i = 1; j-i >= 0; i++) {
                SET_BIT(moves_slide[j][line],0,j-i);
                if(GET_BIT(line,0,j-i))
                    break;
            }
        }
}


/* Check if a movement is valid, given a board */
bool check_valid_move(board_t *b, move_t m) {
    move_list_t *legal = NULL;
    uint8_t i;

    /* Search for the move on the legal moves list */
    legal = gen_move_list(b, FALSE);
    for(i = 0; i < legal->size; i++) {
        if(m.src_y == legal->move[i].src_y && m.src_x == legal->move[i].src_x && m.dst_y == legal->move[i].dst_y && m.dst_x == legal->move[i].dst_x && m.promotion == legal->move[i].promotion) {
            clear_move_list(legal);
            return TRUE;
        }
    }

    /* Clear temporary move list */
    clear_move_list(legal);

    /* If we reach here, the move was not on the legal moves list */
    return FALSE;
}
