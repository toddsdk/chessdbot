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

board.h
Board structure header file. Contains constantes, labels and macros for
manipulating boards, bitboards, bitlines and related structures.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _BOARD_H_
#define _BOARD_H_

#include "main.h"

/* Labels for the board files (columns) */
#define FILE_A 7
#define FILE_B 6
#define FILE_C 5
#define FILE_D 4
#define FILE_E 3
#define FILE_F 2
#define FILE_G 1
#define FILE_H 0
#define FILES 8

/* Labels for the board ranks (rows) */
#define RANK_1 0
#define RANK_2 1
#define RANK_3 2
#define RANK_4 3
#define RANK_5 4
#define RANK_6 5
#define RANK_7 6
#define RANK_8 7
#define RANKS 8

/* Bitboard Rotation Degrees */
#define ROT_0 0
#define ROT_45 1
#define ROT_90 2
#define ROT_315 3
#define ROTATIONS 4

/* Coordinates, used in bitboard rotations */
#define Y 0
#define X 1
#define COORDS 2

#define BOARD_CENTER 0x0000003c3c000000ULL
#define BLACK_SQUARES 0x55AA55AA55AA55AAULL
#define WHITE_SQUARES 0xAA55AA55AA55AA55ULL

/* End games status */
#define NO_MATE 0
#define CHECK_MATE 1
#define STALE_MATE 2
#define REPETITION 3
#define FIFTY_MOVES 4
#define TWO_KINGS 5

/* Color sides */
#define COLOR_BLACK 0
#define COLOR_WHITE 1
#define COLORS 2

/* Piece types */
#define PAWN 0
#define BISHOP 1
#define KNIGHT 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define PIECES 6

/* When a pawn doesn't promotes */
#define NO_PROMOTION 0

/* Sides of castling */
#define QSIDE 0
#define KSIDE 1
#define CASTLE_SIDES 2

/* Castle rights */
/* Castle rights mask === (K(1 bit).Q(1 bit).k(1 bit).q(1 bit)) */
#define CASTLE_KSIDE_WHITE 0x8
#define CASTLE_QSIDE_WHITE 0x4
#define CASTLE_KSIDE_BLACK 0x2
#define CASTLE_QSIDE_BLACK 0x1
#define CASTLE_WHITE (CASTLE_KSIDE_WHITE|CASTLE_QSIDE_WHITE)
#define CASTLE_BLACK (CASTLE_KSIDE_BLACK|CASTLE_QSIDE_BLACK)
#define CASTLE_ALL (CASTLE_WHITE|CASTLE_BLACK)
#define CAN_CASTLE(x,s,c) ((x) & ((s) ? ((s) == CASTLE_SIDES ? ((c) ? CASTLE_WHITE : CASTLE_BLACK) : ((c) ? CASTLE_KSIDE_WHITE : CASTLE_KSIDE_BLACK)) : ((c) ? CASTLE_QSIDE_WHITE : CASTLE_QSIDE_BLACK)))
#define CLEAR_CASTLE(x,s,c) (x) &= ~((s) ? ((s) == CASTLE_SIDES ? ((c) ? CASTLE_WHITE : CASTLE_BLACK) : ((c) ? CASTLE_KSIDE_WHITE : CASTLE_KSIDE_BLACK)) : ((c) ? CASTLE_QSIDE_WHITE : CASTLE_QSIDE_BLACK))

/* Flags for Castling */
#define WHITE_CASTLED 0x2
#define BLACK_CASTLED 0x1

/* En passant rights */
/* En passant rights mask === (valid(1 bit).file(3 bit)) */
#define ENPASSANT_GET_FILE(x) ((x)&0x07)
#define ENPASSANT_GET_VALID(x) (((x)&0x08)>>3)
#define ENPASSANT_SET_FILE(x,f) (x) = (((x)&0x08)|((f)&0x07))
#define ENPASSANT_SET_VALID(x,v) (x) = ((((v)<<3)&0x08)|((x)&0x07))
/*#define ENPASSANT_SET_FILE(x,f) (x) |= ((f)&0x07)
#define ENPASSANT_SET_VALID(x,v) (x) |= (((v)<<3)&0x08)
*/

/* Bitboards manipulation */
#define CLEAR_BIT(b,y,x) (b) &= ~(0x01ULL << ((y)*8+(x)))
#define SET_BIT(b,y,x) (b) |= (0x01ULL << ((y)*8+(x)))
#define GET_BIT(b,y,x) (0x01ULL & ((b)>>(((y)*8)+(x))))
#define GET_LINE(b,y) (0x0FFULL & (b)>>((y)*8))

/* Defines for Linux */
#if defined(Linux)
/* First bit (least significant) of a bitboard */
#define FIRST_BIT(x) (ffsll(x)-1)
/* First bit (least significant) of a bitline */
#define FIRST_BIT_LINE(x) FIRST_BIT(x)
/* Defines for Mac OS X */
#elif defined(Darwin)
/* First bit (least significant) of a bitboard */
#define FIRST_BIT(x) ((x) & 0xFFFFFFFFULL ? (ffs(x)-1) : ((x) & 0xFFFFFFFF00000000ULL ? (ffs((x)>>32)+31) : -1))
/* First bit (least significant) of a bitline */
#define FIRST_BIT_LINE(x) (ffs(x)-1)
#else
#define FIRST_BIT(x) (ffsll(x)-1)
#define FIRST_BIT_LINE(x) FIRST_BIT(x)
#endif

/* Bitboard and Bitline type, used all over the place */
typedef uint64_t bitboard_t;
typedef uint8_t bitline_t;

/* Board structure */
typedef struct {
    bitboard_t bitboard[COLORS][PIECES]; /* Bitboards of all pieces and colors*/
    bitboard_t rotation[COLORS+1][ROTATIONS]; /* 4 bitboard rotations */
    bitboard_t hash; /* hash key to identify a unique board */
    uint8_t castle : 4; /* Flags of castle rights */
    uint8_t enpassant : 4; /* Flags of enpassant rights */
    uint16_t onmove : 1;  /* Color side to play */
    uint16_t hm : 6; /* Number of half-moves */
    uint16_t fm : 9; /* Numver of full-moves*/
    uint8_t castled; /* Flags indicating wheter a side has castled or not */
} board_t; /* 8 x 25 + 4 = 204 bytes */

/* Arrays and variables used by other modules */
extern const bitboard_t file[FILES];
extern const bitboard_t rank[RANKS];
extern const uint8_t rot_map[ROTATIONS][RANKS][FILES][COORDS], unrot_map[ROTATIONS][RANKS][FILES][COORDS];
extern uint64_t zobrist_piece[COLORS][PIECES][RANKS][FILES], zobrist_castle[16], zobrist_enpassant[16], zobrist_white_onmove;
extern const bitline_t rot_mask_45[8][8], rot_mask_315[8][8];

/* Function prototypes */
board_t *set_board(char *fen);
void clear_board(board_t *b);
bool place_pieces(board_t *b, char *pp);
bool set_castle(board_t *b, char *castle);
bool set_enpassant(board_t *b, char *enpassant);
bitboard_t rotate_bitboard(bitboard_t orig, uint8_t rot);
void init_zobrist_keys(void);
uint64_t rand64(void);
uint8_t end(board_t *b);
bool check(board_t *b, bitboard_t king, uint8_t color);
uint8_t mate_or_stale(board_t *b);
bool fifty_moves(board_t *b);
bool two_kings(board_t *b);
bool repetition(board_t *b);
bool evaluate_draw(board_t *b);
inline uint8_t count(bitboard_t bits);
void print_board(board_t *b);
char *print_fen(board_t *b);

#endif
