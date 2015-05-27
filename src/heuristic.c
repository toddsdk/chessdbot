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

heuristic.c
Heuristic module. Contains the Function of Static Analysis for the leaf nodes of
 the Minimax search tree, and also all of it's heuristic components.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "heuristic.h"
#include "moves.h"
#include "levels.h"

static bitboard_t king_distance[RANKS][FILES][8];
static uint8_t dist[RANKS][FILES][RANKS][FILES];
static uint8_t material_value[PIECES];

/* Static Evaluation Function */
int32_t heuristic(board_t *b, uint8_t onmove) {
    int32_t result = 0;

    result += material(b, onmove) * config->factor_material;
    result += development(b, onmove) * config->factor_development;
    result += pawn(b, onmove) * config->factor_pawn;
    result += bishop(b, onmove) * config->factor_bishop;
    result += king(b, onmove) * config->factor_king;
    result += knight(b, onmove) * config->factor_knight;
    result += queen(b, onmove) * config->factor_queen;
    result += rook(b, onmove) * config->factor_rook;

    return result;
}

/* Material Evaluation */
int32_t material(board_t *b, uint8_t onmove) {
    uint8_t color, piece;
    int32_t result = 0, sig;

    /* For each color, evaluate the material advantage */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        sig = (color == onmove) ? 1 : -1;
        /* For each piece type, compute it's value */
        for(piece = PAWN; piece < PIECES; piece++)
            result += count(b->bitboard[color][piece]) * sig * material_value[piece];
    }
    return result;
}

/* Development heuristic */
/* early castle,                 Good
 * early queen and rook movement,         Bad
 * early bishop and knight stuckness,        Bad
 * early king movement unless it's a castle.     Bad */
int32_t development(board_t *b, uint8_t onmove) {
    uint8_t color, first_rank;
    int32_t result = 0, sig;

    /* If we are in the opening phase... */
    if(b->fm <= 10) {
        /* For each color side, let's evaluate the pieces development*/
        for(color = COLOR_BLACK; color < COLORS; color++) {
            sig = color == onmove ? 1 : -1;
            first_rank = color ? RANK_1 : RANK_8;

            /* If we have an early moving queen, bad for us */
            if(!(b->bitboard[color][QUEEN] & rank[first_rank] & file[FILE_D]))
                result += config->bonus_early_queen_move * sig;

            /* If we have a stuck bishop, bad for us */
            if(b->bitboard[color][BISHOP] & rank[first_rank] & (file[FILE_C]|file[FILE_F]))
                result += config->bonus_early_bishop_stuck * sig;

            /* If we have a stuck knight, bad for us */
            if(b->bitboard[color][KNIGHT] & rank[first_rank] & (file[FILE_B]|file[FILE_G]))
                result += config->bonus_early_knight_stuck * sig;

            /* If we cannot castle anymore */
            if(!CAN_CASTLE(b->castle, CASTLE_SIDES, color)) {
                /* If we have castled, let's get a bonus! */
                if(b->castled & (color ? WHITE_CASTLED : BLACK_CASTLED))
                    result += config->bonus_has_castled * sig;
                else
                    result += config->bonus_hasnt_castled * sig;
            }
        }
    }

    return result;
}

/* Pawn Heuristic components */
int32_t pawn(board_t *b, uint8_t onmove) {
    int32_t result = 0;

    result += passed_pawn(b, onmove);
    result += isolated_pawn(b, onmove);
    result += backward_pawn(b, onmove);
    result += doubled_pawn(b, onmove);

    return result;
}

/* Passed Pawns */
int32_t passed_pawn(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, r, color;
    bitboard_t pawns, mask;

    for(color = COLOR_BLACK; color < COLORS; color++) {
        pawns = b->bitboard[color][PAWN];
        for(src = 0; (src = FIRST_BIT(pawns)) != -1; CLEAR_BIT(pawns, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            mask = 0;
            if(color == COLOR_WHITE) {
                for(r = src_y + 1; r < RANKS; r++)
                    mask |= rank[r];
            } else if(color == COLOR_BLACK) {
                for(r = src_y - 1; r >= 0 ; r--)
                    mask |= rank[r];
            }
            mask &= (file[src_x-1] | file[src_x] | file[src_x+1]);
            mask &= b->bitboard[!color][PAWN];
            if(!mask)
                result += config->bonus_passed_pawn * (color == onmove ? 1 : -1);
        }
    }

    return result;
}

/* Isolated Pawns */
int32_t isolated_pawn(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, color;
    bitboard_t pawns;

    for(color = COLOR_BLACK; color < COLORS; color++) {
        pawns = b->bitboard[color][PAWN];
        for(src = 0; (src = FIRST_BIT(pawns)) != -1; CLEAR_BIT(pawns, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            if(!(b->bitboard[color][PAWN] & (file[src_x-1] | file[src_x+1])))
                result += config->bonus_isolated_pawn * (color == onmove ? 1 : -1);
        }
    }

    return result;
}

/* Backward Pawns */
int32_t backward_pawn(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, r, color;
    bitboard_t pawns, mask;

    for(color = COLOR_BLACK; color < COLORS; color++) {
        pawns = b->bitboard[color][PAWN];
        for(src = 0; (src = FIRST_BIT(pawns)) != -1; CLEAR_BIT(pawns, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            mask = 0;
            if(color == COLOR_WHITE) {
                for(r = src_y - 1; r >= 0 ; r--)
                    mask |= rank[r];
            } else if(color == COLOR_BLACK) {
                for(r = src_y + 1; r < RANKS; r++)
                    mask |= rank[r];
            }
            mask &= (file[src_x-1] | file[src_x+1]);
            mask &= b->bitboard[color][PAWN];
            if(!mask)
                result += config->bonus_backward_pawn * (color == onmove ? 1 : -1);
        }
    }

    return result;
}

/* Doubled and tripled pawns */
int32_t doubled_pawn(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t f, n, color;

    for(color = COLOR_BLACK; color < COLORS; color++) {
        for(f = FILE_H; f < FILES; f++) {
            n = count(b->bitboard[color][PAWN] & file[f]);
            if(n == 2) {
                result += config->bonus_doubled_pawn * (color == onmove ? 1 : -1);
            } else if(n >= 3) {
                result += config->bonus_tripled_pawn * (color == onmove ? 1 : -1);
            }
        }
    }

    return result;
}

/* Bishop Heuristic components */
int32_t bishop(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, color;
    bitboard_t bishops, pawns;

    /* For each color, evaluate the bishops */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        bishops = b->bitboard[color][BISHOP];
        /* For each bishop... */
        for(src = 0; (src = FIRST_BIT(bishops)) != -1; CLEAR_BIT(bishops, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;

            /* Compute the board control */
            result += control(b, color, BISHOP, src_y, src_x) * (color == onmove ? 1 : -1);
        }

        /* Doubled bishops */
        bishops = b->bitboard[color][BISHOP];
        if(count(bishops) == 2)
            result += config->bonus_doubled_bishop * (color == onmove ? 1 : -1);

        /* Fianchetto */
        pawns = b->bitboard[color][PAWN];
        if((color == COLOR_WHITE &&
           (((bishops & rank[RANK_2] & file[FILE_G]) && (pawns & (rank[RANK_2]|rank[RANK_3]) & file[FILE_G])) ||
            ((bishops & rank[RANK_2] & file[FILE_B]) && (pawns & (rank[RANK_2]|rank[RANK_3]) & file[FILE_B])) ||
            ((bishops & rank[RANK_3] & file[FILE_H]) && (pawns & rank[RANK_2] & file[FILE_G])) ||
            ((bishops & rank[RANK_3] & file[FILE_A]) && (pawns & rank[RANK_2] & file[FILE_B])))) ||
           (color == COLOR_BLACK &&
           (((bishops & rank[RANK_7] & file[FILE_G]) && (pawns & (rank[RANK_7]|rank[RANK_6]) & file[FILE_G])) ||
            ((bishops & rank[RANK_7] & file[FILE_B]) && (pawns & (rank[RANK_7]|rank[RANK_6]) & file[FILE_B])) ||
            ((bishops & rank[RANK_6] & file[FILE_H]) && (pawns & rank[RANK_7] & file[FILE_G])) ||
            ((bishops & rank[RANK_6] & file[FILE_A]) && (pawns & rank[RANK_7] & file[FILE_B])))))
            result += config->bonus_fianchetto_bishop * (color == onmove ? 1 : -1);
    }

    return result;
}

/* King Heuristic components */
int32_t king(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, color;
    bitboard_t kings;

    /* For each color, evaluate the kings */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        kings = b->bitboard[color][KING];
        /* For each King... */
        for(src = 0; (src = FIRST_BIT(kings)) != -1; CLEAR_BIT(kings, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            /* Compute the board control */
            result += control(b, color, KING, src_y, src_x) * (color == onmove ? 1 : -1);
        }
    }

    return result;
}

/* Knight Heuristic components */
int32_t knight(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, dst_y, dst_x, color, i, j;
    bitboard_t knights, pawns, holes;

    /* For each color, evaluate the knights */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        knights = b->bitboard[color][KNIGHT];
        /* For each Knight... */
        for(src = 0; (src = FIRST_BIT(knights)) != -1; CLEAR_BIT(knights, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            /* Compute the board control */
            result += control(b, color, KNIGHT, src_y, src_x) * (color == onmove ? 1 : -1);
        }

        /* Is there a knight on a board edge? */
        if(b->bitboard[color][KNIGHT] & (rank[RANK_1]|rank[RANK_8]|file[FILE_A]|file[FILE_H]))
            result += config->bonus_knight_on_edge * (color == onmove ? 1 : -1);

        /* Bonus for knights in pawn holes */
        holes = 0;
        /* Get the squares controled by the enemy pawns */
        for(pawns = b->bitboard[!color][PAWN]; (src = FIRST_BIT(pawns)) != -1; CLEAR_BIT(pawns, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            for(i = (color ? -1 : 1), j = -1; j <= 1; j += 2) {
                dst_y = src_y + i;
                dst_x = src_x + j;
                if(dst_y <= RANK_8 && dst_y >= RANK_1 && dst_x <= FILE_A && dst_x >= FILE_H)
                    SET_BIT(holes, dst_y, dst_x);
            }
        }
        /* Now get the squares in front of each enemy pawn */
        pawns = b->bitboard[!color][PAWN];
        pawns = color ? ((pawns >> 8) & ~(b->rotation[COLORS][ROT_0])) : ((pawns << 8) & ~(b->rotation[COLORS][ROT_0]));
        /* The holes are squares in front of an enemy pawn that are not being attacked by any enemy pawn */
        holes = ~(holes) & pawns;
        /* Is there a knight on a pawn hole? */
        if(b->bitboard[color][KNIGHT] & holes)
            result += config->bonus_knight_on_hole * (color == onmove ? 1 : -1);
    }

    return result;
}

/* Queen Heuristic components */
int32_t queen(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, color;
    bitboard_t queens;

    /* For each color, evaluate the queens */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        queens = b->bitboard[color][QUEEN];
        /* For each Queen... */
        for(src = 0; (src = FIRST_BIT(queens)) != -1; CLEAR_BIT(queens, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            /* Compute the board control */
            result += control(b, color, QUEEN, src_y, src_x) * (color == onmove ? 1 : -1);

            /* If there are no friendly pawns on this file */
            if(!(b->bitboard[color][PAWN] & file[src_x])) {
                /* If there are no enemy pawns on this file */
                if(!(b->bitboard[!color][PAWN] & file[src_x])) {
                    /* Queen on open file */
                    result += config->bonus_queen_open_file * (color == onmove ? 1 : -1);
                } else {
                    /* Queen on half-open file */
                    result += config->bonus_queen_halfopen_file * (color == onmove ? 1 : -1);
                }
            }
        }
    }

    return result;
}

/* Rook Heuristic components */
int32_t rook(board_t *b, uint8_t onmove) {
    int32_t result = 0;
    int8_t src, src_y, src_x, color;
    bitboard_t rooks;

    /* For each color, evaluate the rooks */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        rooks = b->bitboard[color][ROOK];
        /* For each Rook... */
        for(src = 0; (src = FIRST_BIT(rooks)) != -1; CLEAR_BIT(rooks, src_y, src_x)) {
            src_y = src/8;
            src_x = src%8;
            /* Compute the board control */
            result += control(b, color, ROOK, src_y, src_x) * (color == onmove ? 1 : -1);
            /* If there are no friendly pawns on this file */
            if(!(b->bitboard[color][PAWN] & file[src_x])) {
                /* If there are no enemy pawns on this file */
                if(!(b->bitboard[!color][PAWN] & file[src_x])) {
                    /* Rook on open file */
                    result += config->bonus_rook_open_file * (color == onmove ? 1 : -1);
                } else {
                    /* Rook on half-open file */
                    result += config->bonus_rook_halfopen_file * (color == onmove ? 1 : -1);
                }
            }
        }
    }

    return result;
}

/* Determines amount of board control for a specific piece */
int32_t control(board_t *b, uint8_t onmove, uint8_t piece, int8_t src_y, int8_t src_x) {
    int8_t dst, dst_y, dst_x, rot_y, rot_x, unrot_y, unrot_x;
    bitboard_t ctl = 0, to;
    bitline_t line;
    int32_t result = 0;

    /* Sets the piece's valid moves on a bitboard */
    switch(piece) {
    case BISHOP:
        rot_y = rot_map[ROT_45][src_y][src_x][Y];
        rot_x = rot_map[ROT_45][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_45], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_45[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_45], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_45][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_45][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
        rot_y = rot_map[ROT_315][src_y][src_x][Y];
        rot_x = rot_map[ROT_315][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_315], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_315[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_315], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_315][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_315][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
    break;
    case KING:
        to = moves_king[src_y][src_x] & ~(b->rotation[onmove][ROT_0]);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
            dst_y = dst/8;
            dst_x = dst%8;
            SET_BIT(ctl, dst_y, dst_x);
        }
    break;
    case KNIGHT:
        to = moves_knight[src_y][src_x] & ~(b->rotation[onmove][ROT_0]);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, dst_y, dst_x)) {
            dst_y = dst/8;
            dst_x = dst%8;
            SET_BIT(ctl, dst_y, dst_x);
        }
    break;
    case QUEEN:
        rot_y = rot_map[ROT_0][src_y][src_x][Y];
        rot_x = rot_map[ROT_0][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_0], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_0], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_0][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_0][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
        rot_y = rot_map[ROT_90][src_y][src_x][Y];
        rot_x = rot_map[ROT_90][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_90], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_90], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_90][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_90][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
        rot_y = rot_map[ROT_45][src_y][src_x][Y];
        rot_x = rot_map[ROT_45][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_45], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_45[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_45], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_45][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_45][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
        rot_y = rot_map[ROT_315][src_y][src_x][Y];
        rot_x = rot_map[ROT_315][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_315], rot_y);
        to = (moves_slide[rot_x][line] & rot_mask_315[rot_y][rot_x]) & ~GET_LINE(b->rotation[onmove][ROT_315], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_315][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_315][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
    break;
    case ROOK:
        rot_y = rot_map[ROT_0][src_y][src_x][Y];
        rot_x = rot_map[ROT_0][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_0], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_0], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_0][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_0][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
        rot_y = rot_map[ROT_90][src_y][src_x][Y];
        rot_x = rot_map[ROT_90][src_y][src_x][X];
        line = GET_LINE(b->rotation[COLORS][ROT_90], rot_y);
        to = moves_slide[rot_x][line] & ~GET_LINE(b->rotation[onmove][ROT_90], rot_y);
        for(dst = 0; (dst = FIRST_BIT(to)) != -1; CLEAR_BIT(to, 0, dst_x)) {
            dst_x = dst%8;
            unrot_y = unrot_map[ROT_90][rot_y][dst_x][Y];
            unrot_x = unrot_map[ROT_90][rot_y][dst_x][X];
            SET_BIT(ctl, unrot_y, unrot_x);
        }
    break;
    default:
        return 0;
    }

    /* Compute center control */
    result += config->bonus_center_control * count(ctl & BOARD_CENTER);

    /* Compute friendly king protection (distance lesser or equal to 2) */
    dst = FIRST_BIT(b->bitboard[onmove][KING]);
    dst_y = dst/8;
    dst_x = dst%8;
    result += count(ctl & king_distance[dst_y][dst_x][2]);

    /* Compute enemy king menace (distance lesser or equal to 2) */
    dst = FIRST_BIT(b->bitboard[!onmove][KING]);
    dst_y = dst/8;
    dst_x = dst%8;
    result += count(ctl & king_distance[dst_y][dst_x][2]);

    /* Compute number of controlled positions */
    result += count(ctl);

    return result;
}

/* Initialize useful distance matrices */
void precompute_distances(void) {
    uint8_t src, src_y, src_x, dst, dst_y, dst_x, d, i, dx, dy;

    /* Fill the king's distances with zeros */
    for(src = 0; src < 64; src++)
        for(d = 0; d < 8; d++)
            king_distance[src/8][src%8][d] = 0;

    /* Calculate the distances matrix (euclidean distance) */
    for(src = 0; src < 64; src++)
        for(dst = 0; dst < 64; dst++) {
            src_y = src/8;
            src_x = src%8;
            dst_y = dst/8;
            dst_x = dst%8;
            dx = src_x > dst_x ? src_x - dst_x : dst_x - src_x;
            dy = src_y > dst_y ? src_y - dst_y : dst_y - src_y;
            dist[src_y][src_x][dst_y][dst_x] = MAX(dx,dy);
        }

    /* Set the bitboards accordingly with king moves */
    for(src = 0; src < 64; src++)
        for(dst = 0; dst < 64; dst++) {
            src_y = src/8;
            src_x = src%8;
            dst_y = dst/8;
            dst_x = dst%8;
            SET_BIT(king_distance[src_y][src_x][dist[src_y][src_x][dst_y][dst_x]], dst_y, dst_x);
        }

    /* Add the lesser distances to each bitboard */
    for(src = 0; src < 64; src++)
        for(d = 0; d < 8; d++)
            for(i = 0; i < d; i++) {
                src_y = src/8;
                src_x = src%8;
                king_distance[src_y][src_x][d] |= king_distance[src_y][src_x][i];
            }

    material_value[PAWN] = config->pawn_val;
    material_value[BISHOP] = config->bishop_val;
    material_value[KNIGHT] = config->knight_val;
    material_value[ROOK] = config->rook_val;
    material_value[QUEEN] = config->queen_val;
    material_value[KING] = config->king_val;
}
