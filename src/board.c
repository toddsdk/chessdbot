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

board.c
Board structure manipulation. Contains functions and arrays to create and use
the 'board_t' structure.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "board.h"
#include "moves.h"
#include "history.h"

/* Masks for the board files (columns) */
const bitboard_t file[FILES] = {
0x0101010101010101ULL,
0x0202020202020202ULL,
0x0404040404040404ULL,
0x0808080808080808ULL,
0x1010101010101010ULL,
0x2020202020202020ULL,
0x4040404040404040ULL,
0x8080808080808080ULL
};

/* Masks for the board ranks (rows) */
const bitboard_t rank[RANKS] = {
0x00000000000000FFULL,
0x000000000000FF00ULL,
0x0000000000FF0000ULL,
0x00000000FF000000ULL,
0x000000FF00000000ULL,
0x0000FF0000000000ULL,
0x00FF000000000000ULL,
0xFF00000000000000ULL
};

/* Rotation maps to rotate bitboards [on the form (y,x)] */
const uint8_t rot_map[ROTATIONS][RANKS][FILES][COORDS] = {
{ /* 0 -> 0 */
{{0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7}},
{{1,0},{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7}},
{{2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7}},
{{3,0},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6},{3,7}},
{{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7}},
{{5,0},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},{5,7}},
{{6,0},{6,1},{6,2},{6,3},{6,4},{6,5},{6,6},{6,7}},
{{7,0},{7,1},{7,2},{7,3},{7,4},{7,5},{7,6},{7,7}}},
{ /* 0 -> 45 */
{{0,0},{7,1},{6,2},{5,3},{4,4},{3,5},{2,6},{1,7}},
{{1,0},{0,1},{7,2},{6,3},{5,4},{4,5},{3,6},{2,7}},
{{2,0},{1,1},{0,2},{7,3},{6,4},{5,5},{4,6},{3,7}},
{{3,0},{2,1},{1,2},{0,3},{7,4},{6,5},{5,6},{4,7}},
{{4,0},{3,1},{2,2},{1,3},{0,4},{7,5},{6,6},{5,7}},
{{5,0},{4,1},{3,2},{2,3},{1,4},{0,5},{7,6},{6,7}},
{{6,0},{5,1},{4,2},{3,3},{2,4},{1,5},{0,6},{7,7}},
{{7,0},{6,1},{5,2},{4,3},{3,4},{2,5},{1,6},{0,7}}},
{ /* 0 -> 90 */
{{7,0},{6,0},{5,0},{4,0},{3,0},{2,0},{1,0},{0,0}},
{{7,1},{6,1},{5,1},{4,1},{3,1},{2,1},{1,1},{0,1}},
{{7,2},{6,2},{5,2},{4,2},{3,2},{2,2},{1,2},{0,2}},
{{7,3},{6,3},{5,3},{4,3},{3,3},{2,3},{1,3},{0,3}},
{{7,4},{6,4},{5,4},{4,4},{3,4},{2,4},{1,4},{0,4}},
{{7,5},{6,5},{5,5},{4,5},{3,5},{2,5},{1,5},{0,5}},
{{7,6},{6,6},{5,6},{4,6},{3,6},{2,6},{1,6},{0,6}},
{{7,7},{6,7},{5,7},{4,7},{3,7},{2,7},{1,7},{0,7}}},
{ /* 0 -> 315 */
{{0,0},{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7}},
{{1,0},{2,1},{3,2},{4,3},{5,4},{6,5},{7,6},{0,7}},
{{2,0},{3,1},{4,2},{5,3},{6,4},{7,5},{0,6},{1,7}},
{{3,0},{4,1},{5,2},{6,3},{7,4},{0,5},{1,6},{2,7}},
{{4,0},{5,1},{6,2},{7,3},{0,4},{1,5},{2,6},{3,7}},
{{5,0},{6,1},{7,2},{0,3},{1,4},{2,5},{3,6},{4,7}},
{{6,0},{7,1},{0,2},{1,3},{2,4},{3,5},{4,6},{5,7}},
{{7,0},{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7}}},
};

/* Rotation maps to UNrotate bitboards [on the form (y,x)] */
const uint8_t unrot_map[ROTATIONS][8][8][COORDS] = {
{ /* 0 -> 0 */
{{0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7}},
{{1,0},{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7}},
{{2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7}},
{{3,0},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6},{3,7}},
{{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7}},
{{5,0},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},{5,7}},
{{6,0},{6,1},{6,2},{6,3},{6,4},{6,5},{6,6},{6,7}},
{{7,0},{7,1},{7,2},{7,3},{7,4},{7,5},{7,6},{7,7}}},
{ /* 45 -> 0 */
{{0,0},{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7}},
{{1,0},{2,1},{3,2},{4,3},{5,4},{6,5},{7,6},{0,7}},
{{2,0},{3,1},{4,2},{5,3},{6,4},{7,5},{0,6},{1,7}},
{{3,0},{4,1},{5,2},{6,3},{7,4},{0,5},{1,6},{2,7}},
{{4,0},{5,1},{6,2},{7,3},{0,4},{1,5},{2,6},{3,7}},
{{5,0},{6,1},{7,2},{0,3},{1,4},{2,5},{3,6},{4,7}},
{{6,0},{7,1},{0,2},{1,3},{2,4},{3,5},{4,6},{5,7}},
{{7,0},{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7}}},
{ /* 90 -> 0 */
{{0,7},{1,7},{2,7},{3,7},{4,7},{5,7},{6,7},{7,7}},
{{0,6},{1,6},{2,6},{3,6},{4,6},{5,6},{6,6},{7,6}},
{{0,5},{1,5},{2,5},{3,5},{4,5},{5,5},{6,5},{7,5}},
{{0,4},{1,4},{2,4},{3,4},{4,4},{5,4},{6,4},{7,4}},
{{0,3},{1,3},{2,3},{3,3},{4,3},{5,3},{6,3},{7,3}},
{{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},{7,2}},
{{0,1},{1,1},{2,1},{3,1},{4,1},{5,1},{6,1},{7,1}},
{{0,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0}}},
{ /* 315 -> 0 */
{{0,0},{7,1},{6,2},{5,3},{4,4},{3,5},{2,6},{1,7}},
{{1,0},{0,1},{7,2},{6,3},{5,4},{4,5},{3,6},{2,7}},
{{2,0},{1,1},{0,2},{7,3},{6,4},{5,5},{4,6},{3,7}},
{{3,0},{2,1},{1,2},{0,3},{7,4},{6,5},{5,6},{4,7}},
{{4,0},{3,1},{2,2},{1,3},{0,4},{7,5},{6,6},{5,7}},
{{5,0},{4,1},{3,2},{2,3},{1,4},{0,5},{7,6},{6,7}},
{{6,0},{5,1},{4,2},{3,3},{2,4},{1,5},{0,6},{7,7}},
{{7,0},{6,1},{5,2},{4,3},{3,4},{2,5},{1,6},{0,7}}},
};

/* Masks used in diagonal rotations (45 and 315 degrees)*/
const bitline_t rot_mask_45[8][8] = {
{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x80},
{0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0xC0,0xC0},
{0x1F,0x1F,0x1F,0x1F,0x1F,0xE0,0xE0,0xE0},
{0x0F,0x0F,0x0F,0x0F,0xF0,0xF0,0xF0,0xF0},
{0x07,0x07,0x07,0xF8,0xF8,0xF8,0xF8,0xF8},
{0x03,0x03,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC},
{0x01,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE}};
const bitline_t rot_mask_315[8][8] = {
{0x01,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE},
{0x03,0x03,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC},
{0x07,0x07,0x07,0xF8,0xF8,0xF8,0xF8,0xF8},
{0x0F,0x0F,0x0F,0x0F,0xF0,0xF0,0xF0,0xF0},
{0x1F,0x1F,0x1F,0x1F,0x1F,0xE0,0xE0,0xE0},
{0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0xC0,0xC0},
{0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x80},
{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}};

/* Zobrist hash keys, used to determine bitboard hashes */
uint64_t zobrist_piece[COLORS][PIECES][RANKS][FILES], zobrist_castle[16], zobrist_enpassant[16], zobrist_white_onmove;

/* Setup a board for a given FEN string */
board_t *set_board(char *fen) {
    board_t *b;
    char fen_pp[128], fen_color, fen_castle[8], fen_enpassant[8];
    uint32_t hm, fm, piece, rot;

    /* Validate and "explode" FEN string components */
    if(sscanf(fen,"%s %c %s %s %d %d", fen_pp, &fen_color, fen_castle, fen_enpassant, &hm, &fm) != 6)
        return NULL;

    /* Ask space for the board structure */
    b = (board_t *) malloc(sizeof(board_t));
    if(b == NULL)
        return NULL;

    /* Fill it with zeros */
    b = memset(b, 0, sizeof(board_t));

    /* Set the side (color) which is the next to play */
    if(fen_color == 'w') {
        b->onmove = COLOR_WHITE;
        b->hash ^= zobrist_white_onmove;
    } else if(fen_color == 'b') {
        b->onmove = COLOR_BLACK;
    } else {
        return NULL;
    }

    /* Put each piece on their own place */
    if(!place_pieces(b, fen_pp))
        return NULL;

    /* Setup castle conditions */
    if(!set_castle(b, fen_castle))
        return NULL;

    /* Let's say that nobody has castled (may not work well for some boards) */
    b->castled = 0;

    /* Setup En-passant conditions */
    if(!set_enpassant(b, fen_enpassant))
        return NULL;

    /* Setup Halve-moves and Full-moves count */
    b->hm = hm;
    b->fm = fm;

    /* Create Zero degree bitboard rotation */
    for(piece = PAWN; piece < PIECES; piece++) {
        b->rotation[COLOR_BLACK][ROT_0] |= b->bitboard[COLOR_BLACK][piece];
        b->rotation[COLOR_WHITE][ROT_0] |= b->bitboard[COLOR_WHITE][piece];
    }

    /* Adds black+white pieces to a same bitboard rotation */
    b->rotation[COLORS][ROT_0] = b->rotation[COLOR_BLACK][ROT_0] | b->rotation[COLOR_WHITE][ROT_0];

    /* Rotate the bitboards and save those rotations (45, 90 and 315 degrees) */
    for(rot = ROT_45; rot < ROTATIONS; rot++) {
        b->rotation[COLOR_BLACK][rot] = rotate_bitboard(b->rotation[COLOR_BLACK][ROT_0], rot);
        b->rotation[COLOR_WHITE][rot] = rotate_bitboard(b->rotation[COLOR_WHITE][ROT_0], rot);
        b->rotation[COLORS][rot] = b->rotation[COLOR_BLACK][rot] | b->rotation[COLOR_WHITE][rot];
    }

    return b;
}

/* Releases the memory used by a board structure */
void clear_board(board_t *b) {
    if(b)
        free(b);
}

/* Set the place of the pieces in a board, based on the FEN string component */
bool place_pieces(board_t *b, char *pp) {
    uint8_t lin = 0, col = 0, color;

    /* For each char... */
    while(*pp) {
        /* In case of a letter, we have a piece */
        if(isalpha(*pp)) {
            /* Determine the color of the piece */
            if(isupper(*pp))
                color = COLOR_WHITE;
            else
                color = COLOR_BLACK;
            /* Which piece is it? Set a bit in the appropriate bitboard */
            switch(tolower(*pp)) {
            case 'k': /* King */
                SET_BIT(b->bitboard[color][KING], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][KING][7-lin][7-col];
                break;
            case 'q': /* Queen */
                SET_BIT(b->bitboard[color][QUEEN], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][QUEEN][7-lin][7-col];
                break;
            case 'b': /* Bishop */
                SET_BIT(b->bitboard[color][BISHOP], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][BISHOP][7-lin][7-col];
                break;
            case 'n': /* Knight */
                SET_BIT(b->bitboard[color][KNIGHT], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][KNIGHT][7-lin][7-col];
                break;
            case 'r': /* Rook */
                SET_BIT(b->bitboard[color][ROOK], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][ROOK][7-lin][7-col];
                break;
            case 'p': /* Pawn */
                SET_BIT(b->bitboard[color][PAWN], 7-lin, 7-col);
                b->hash ^= zobrist_piece[color][PAWN][7-lin][7-col];
                break;
            default:
                return FALSE;
            }
            col++;
        /* If it's a number, jump some squares */
        } else if(isdigit(*pp)) {
            col += atoi(pp);
        /* Get down to the next rank if we got a slash */
        } else if(*pp == '/' && col == 8) {
            lin++;
            col = 0;
        } else {
            return FALSE;
        }
        pp++;
    }

    return TRUE;
}

/* Set castle conditions, accordingly to the FEN string component */
bool set_castle(board_t *b, char *castle) {

    /* Nobody can castle */
    if(!strcmp(castle,"-"))
        b->castle = 0;
    else if(!strcmp(castle,"K"))
        b->castle = CASTLE_KSIDE_WHITE;
    else if(!strcmp(castle,"KQ"))
        b->castle = CASTLE_WHITE;
    else if(!strcmp(castle,"KQk"))
        b->castle = CASTLE_WHITE | CASTLE_KSIDE_BLACK;
    else if(!strcmp(castle,"KQkq"))
        b->castle = CASTLE_ALL;
    else if(!strcmp(castle,"KQq"))
        b->castle = CASTLE_WHITE | CASTLE_QSIDE_BLACK;
    else if(!strcmp(castle,"Kk"))
        b->castle = CASTLE_KSIDE_WHITE | CASTLE_KSIDE_BLACK;
    else if(!strcmp(castle,"Kkq"))
        b->castle = CASTLE_KSIDE_WHITE | CASTLE_BLACK;
    else if(!strcmp(castle,"Kq"))
        b->castle = CASTLE_KSIDE_WHITE | CASTLE_QSIDE_BLACK;
    else if(!strcmp(castle,"Q"))
        b->castle = CASTLE_QSIDE_WHITE;
    else if(!strcmp(castle,"Qk"))
        b->castle = CASTLE_QSIDE_WHITE | CASTLE_KSIDE_BLACK;
    else if(!strcmp(castle,"Qkq"))
        b->castle = CASTLE_QSIDE_WHITE | CASTLE_BLACK;
    else if(!strcmp(castle,"Qq"))
        b->castle = CASTLE_QSIDE_WHITE | CASTLE_QSIDE_BLACK;
    else if(!strcmp(castle,"k"))
        b->castle = CASTLE_KSIDE_BLACK;
    else if(!strcmp(castle,"kq"))
        b->castle = CASTLE_BLACK;
    else if(!strcmp(castle,"q"))
        b->castle = CASTLE_QSIDE_BLACK;
    else
        return FALSE;

    /* XOR the zobrist key of the castle conditions to the board hash*/
    b->hash ^= zobrist_castle[b->castle];

    return TRUE;
}

/* Set en-passant conditions, accordingly to the FEN string component */
bool set_enpassant(board_t *b, char *enpassant) {
    /* If the string is not a dash, we have some valid en-passant */
    if(strcmp(enpassant,"-")) {
        ENPASSANT_SET_VALID(b->enpassant, TRUE);

        /* If first char isn't letter, or second isn't digit, error */
        if(!isalpha(enpassant[0]) || !isdigit(enpassant[1]) || strlen(enpassant) > 2)
            return FALSE;

        /* Check enpassant rank */
        if(enpassant[1] != '3' && enpassant[1] != '6')
            return FALSE;

        /* Set enpassant file number */
        if(enpassant[0] >= 'a' && enpassant[0] <= 'h')
            ENPASSANT_SET_FILE(b->enpassant, 'h' - enpassant[0]);
        else
            return FALSE;

    } else {
        ENPASSANT_SET_VALID(b->enpassant, FALSE);
        ENPASSANT_SET_FILE(b->enpassant, 0);
    }

    /* XOR the zobrist key of the en-passant conditions to the board hash*/
    b->hash ^= zobrist_enpassant[b->enpassant];

    return TRUE;
}

/* Rotates a given bitboard to a given degree of rotation */
bitboard_t rotate_bitboard(bitboard_t orig, uint8_t rot) {
    int8_t pos, y, x;
    bitboard_t rotated = 0;

    /* For each bit in the bitboard */
    for(pos = 0; (pos = FIRST_BIT(orig)) != -1; CLEAR_BIT(orig, y, x)) {
        /* Get it's coordinates */
        y = pos/8;
        x = pos%8;
       /* Set a bit on the rotated bitboard accordingly with rotation maps */
        SET_BIT(rotated, rot_map[rot][y][x][Y], rot_map[rot][y][x][X]);
    }

    return rotated;
}

/* Initialize zobrist hash keys for every board component */
void init_zobrist_keys(void) {
    uint8_t castle, enpassant, color, piece, y, x;

    /* Set the seed of the random number generator */
    srand(time(NULL));

    /* Keys of the pieces for each position */
    for(color = COLOR_BLACK; color < COLORS; color++)
        for(piece = PAWN; piece < PIECES; piece++)
            for(y = 0; y < 8; y++)
                for(x = 0; x < 8; x++)
                    zobrist_piece[color][piece][y][x] = rand64();

    /* Keys of the castle conditions */
    for(castle = 0; castle <= CASTLE_ALL; castle++)
        zobrist_castle[castle] = rand64();

    /* Keys of the en-passant conditions */
    enpassant = 0;
    ENPASSANT_SET_VALID(enpassant, TRUE);
    for(x = 0; x < 8; x++) {
        ENPASSANT_SET_FILE(enpassant, x);
        zobrist_enpassant[enpassant] = rand64();
    }
    ENPASSANT_SET_VALID(enpassant, FALSE);
    ENPASSANT_SET_FILE(enpassant, 0);
    zobrist_enpassant[enpassant] = rand64();

    /* Key of the white side on move */
    zobrist_white_onmove = rand64();
}

/* Returns a random 64bit number */
uint64_t rand64(void) {
    return (uint64_t) rand() ^ ((uint64_t)rand() << 16)
      ^ ((uint64_t)rand() << 32) ^ ((uint64_t)rand() << 48);
}

/* Checks the end game conditions */
uint8_t end(board_t *b) {
    uint8_t status;

    /* Checkmate or Stalemate */
    if((status = mate_or_stale(b)) != NO_MATE) {
        return status;
    /* Three move repetition */
    } else if(repetition(b)) {
        return REPETITION;
    /* Fifty moves rule */
    } else if(fifty_moves(b)) {
        return FIFTY_MOVES;
    /* Only the kings on the board*/
    } else if(two_kings(b)) {
        return TWO_KINGS;
    /* Not end of the game */
    } else {
        return FALSE;
    }
}

/* Check if a given king (bitboard with the king set) is being checked by the
 * opponent color (opp) */
bool check(board_t *b, bitboard_t king, uint8_t opp) {
    int8_t src, src_y, src_x, dst_y, dst_x, i, j, rot;
    bitboard_t rotated;
    bitline_t line, mask;

    for(; (src = FIRST_BIT(king)) != -1; CLEAR_BIT(king, src_y, src_x)) {
        src_y = src/8;
        src_x = src%8;

        /* Opponent's king range */
        if(moves_king[src_y][src_x] & b->bitboard[opp][KING])
            return TRUE;

        /* Opponent's knights range */
        if(moves_knight[src_y][src_x] & b->bitboard[opp][KNIGHT])
            return TRUE;

        /* Opponent's sliding pieces */
        for(rot = ROT_0; rot < ROTATIONS; rot++) {
            dst_y = rot_map[rot][src_y][src_x][Y];
            dst_x = rot_map[rot][src_y][src_x][X];
            line = GET_LINE(b->rotation[COLORS][rot], dst_y);

            if(rot == ROT_0 || rot == ROT_90) {
                /* Queens and Rooks horizontal and vertical ranges */
                rotated = rotate_bitboard(b->bitboard[opp][ROOK] | b->bitboard[opp][QUEEN], rot);
                if(moves_slide[dst_x][line] & GET_LINE(rotated, dst_y))
                    return TRUE;
            } else if(rot == ROT_45 || rot == ROT_315) {
                /* Queens and Bishops diagonal ranges */
                rotated = rotate_bitboard(b->bitboard[opp][BISHOP] | b->bitboard[opp][QUEEN], rot);
                mask = (rot == ROT_45) ? rot_mask_45[dst_y][dst_x] : rot_mask_315[dst_y][dst_x];
                if((moves_slide[dst_x][line] & mask) & GET_LINE(rotated, dst_y))
                    return TRUE;
            }
        }

        /* Opponent's pawns range */
        for(i = (opp ? -1 : 1), j = -1; j <= 1; j += 2) {
            dst_y = src_y + i;
            dst_x = src_x + j;
            if(dst_y <= 7 && dst_y >= 0 && dst_x <= 7 && dst_x >= 0)
                if(GET_BIT(b->bitboard[opp][PAWN], dst_y, dst_x))
                    return TRUE;
        }
    }
    return FALSE;
}

/* Check if we are in a checkmate or stalemate */
uint8_t mate_or_stale(board_t *b) {
    uint8_t i, onmove = b->onmove;
    bool in_check = TRUE;
    move_list_t *list = gen_move_list(b, FALSE);

    /* Try all the next moves */
    for(i = 0; i < list->size && in_check; i++) {
        move(b, list->move[i]);
        if(!check(b, b->bitboard[onmove][KING], !onmove))
            in_check = FALSE;
        unmove(b);
    }

    /* Clears the space previously allocated for the move list */
    clear_move_list(list);

    /* If after all those moves we would still be in check, it's a mate.
     * Otherwise, it's a stalemate (draw) */
    if(in_check) {
        if(check(b, b->bitboard[onmove][KING], !onmove))
            return CHECK_MATE;
        else
            return STALE_MATE;
    } else {
        return NO_MATE;
    }
}

/* Three full-moves repetition */
bool repetition(board_t *b) {
    int8_t same = 1, i;
    board_t *h;

    /* Walk over the history, looking for early matching hashes (equal boards)*/
    for(i = 0; (h = peek_history_board(i)) != NULL; i++) {
        if(h->hash == b->hash) {
            same++;
            /* If it matches 3 times, we have a draw */
            if(same == 3)
                return TRUE;
        }
    }

    return FALSE;
}

/* Fifty moves rule */
bool fifty_moves(board_t *b) {
    /* If the Halve-moves count is greater or equal to 50, we have a draw */
    if(b->hm >= 50)
        return TRUE;
    else
        return FALSE;
}

/* Check if we have only the kings on a board */
bool two_kings(board_t *b) {
    uint8_t piece;
    for(piece = PAWN; piece < KING; piece++)
        if(b->bitboard[COLOR_WHITE][piece] || b->bitboard[COLOR_BLACK][piece])
            return FALSE;

    return TRUE;
}

/* Determines if the engine agrees with the draw offer or not */
bool evaluate_draw(board_t *b) {
    uint8_t quantity[COLORS][PIECES], total[COLORS], piece, color;

    /* Compute quantitys of each kind of piece */
    for(color = COLOR_BLACK; color < COLORS; color++) {
        total[color] = 0;
        for(piece = PAWN; piece < PIECES; piece++) {
            quantity[color][piece] = count(b->bitboard[color][piece]);
            total[color] += quantity[color][piece];
        }
    }
    /* Insufficient material conditios */
    if(total[COLOR_WHITE] == 2 && total[COLOR_BLACK] == 1) {
        /* King and Bishop vs. King & King and Knight vs. King */
        if(quantity[COLOR_WHITE][BISHOP] == 1 || quantity[COLOR_WHITE][KNIGHT] == 1)
            return TRUE;
    } else if(total[COLOR_BLACK] == 2 && total[COLOR_WHITE] == 1) {
        /* King and Bishop vs. King & King and Knight vs. King */
        if(quantity[COLOR_BLACK][BISHOP] == 1 || quantity[COLOR_BLACK][KNIGHT] == 1)
            return TRUE;
    } else if(total[COLOR_BLACK] == 2 && total[COLOR_WHITE] == 2) {
        /* King and Bishop vs. King and Bishop (Bishops on the same square color*/
        if(quantity[COLOR_BLACK][BISHOP] == 1 && quantity[COLOR_WHITE][BISHOP] == 1)
            if((b->bitboard[COLOR_WHITE][BISHOP] & BLACK_SQUARES &&
               b->bitboard[COLOR_BLACK][BISHOP] & BLACK_SQUARES) ||
               (b->bitboard[COLOR_WHITE][BISHOP] & WHITE_SQUARES &&
               b->bitboard[COLOR_BLACK][BISHOP] & WHITE_SQUARES))
                return TRUE;
    /* King vs. King */
    } else if(total[COLOR_BLACK] == 1 && total[COLOR_WHITE] == 1) {
        return TRUE;
    }
    return FALSE;
}

/* Returns the number of bits in a bitboard */
inline uint8_t count(bitboard_t bits) {
    uint8_t c, y, x;
    int8_t pos;
    for(c = 0; (pos = FIRST_BIT(bits)) != -1; CLEAR_BIT(bits, y, x)) {
        y = pos/8;
        x = pos%8;
        c++;
    }
    return c;
}

/* Print a board on the screen -- useful for debugging */
void print_board(board_t *b) {
    uint8_t y, x, color, piece, c;
    for(y = 0; y < 8; y++) {
        for(x = 0; x < 8; x++) {
            c = '\0';
            for(color = COLOR_BLACK; color < COLORS; color++) {
                for(piece = PAWN; piece < PIECES; piece++) {
                    if(GET_BIT(b->bitboard[color][piece], y, x)) {
                        switch(piece) {
                        case PAWN:
                            c = 'p';
                            break;
                        case BISHOP:
                            c = 'b';
                            break;
                        case KNIGHT:
                            c = 'n';
                            break;
                        case ROOK:
                            c = 'r';
                            break;
                        case QUEEN:
                            c = 'q';
                            break;
                        case KING:
                            c = 'k';
                            break;
                        }
                        if(color)
                            c = toupper(c);
                        printf("%c", c);
                    }
                }
            }
            if(!c)
                printf(".");
            printf(" ");
        }
        printf("\n");
    }
}


/* Print a FEN board on the screen -- useful for debugging */
char *print_fen(board_t *b) {
    int16_t y, x;
    uint8_t color, piece, c, blank = 0;
    bool castle = FALSE;
    static char buf[80], *a;
    buf[0] = 0;
    a = buf;
    for(y = 7; y >= 0; y--) {
        for(x = 7; x >= 0; x--) {
            c = '\0';
            for(color = COLOR_BLACK; color < COLORS; color++) {
                for(piece = PAWN; piece < PIECES; piece++) {
                    if(GET_BIT(b->bitboard[color][piece], y, x)) {
                        switch(piece) {
                        case PAWN:
                            c = 'p';
                            break;
                        case BISHOP:
                            c = 'b';
                            break;
                        case KNIGHT:
                            c = 'n';
                            break;
                        case ROOK:
                            c = 'r';
                            break;
                        case QUEEN:
                            c = 'q';
                            break;
                        case KING:
                            c = 'k';
                            break;
                        }
                        if(color)
                            c = toupper(c);
                    }
                }
            }
            if(!c) {
                blank++;
                if(x == 0) {
                    sprintf(buf, "%s%d", a, blank);
                    blank = 0;
                }
            } else {
                if(blank > 0) {
                    sprintf(buf, "%s%d", a, blank);
                    blank = 0;
                }
                sprintf(buf, "%s%c", a, c);
            }
        }
        if(y != 0) {
            sprintf(buf, "%s/", a);
        }
    }

    sprintf(buf, "%s %c ", a, b->onmove ? 'w' : 'b');

    if(CAN_CASTLE(b->castle, KSIDE, COLOR_WHITE)) {
        castle = TRUE;
        sprintf(buf, "%sK", a);
    }
    if(CAN_CASTLE(b->castle, QSIDE, COLOR_WHITE)) {
        castle = TRUE;
        sprintf(buf, "%sQ", a);
    }
    if(CAN_CASTLE(b->castle, KSIDE, COLOR_BLACK)) {
        castle = TRUE;
        sprintf(buf, "%sk", a);
    }
    if(CAN_CASTLE(b->castle, QSIDE, COLOR_BLACK)) {
        castle = TRUE;
        sprintf(buf, "%sq", a);
    }
    if(!castle)
        sprintf(buf, "%s-", a);


    sprintf(buf, "%s ", a);
    if(ENPASSANT_GET_VALID(b->enpassant)) {
        sprintf(buf, "%s%c%c", a, 'h' - ENPASSANT_GET_FILE(b->enpassant), b->onmove ? '6':'3');
    } else {
        sprintf(buf, "%s-", a);
    }
    sprintf(buf,"%s %d %d ", a, b->hm, b->fm);

    return a;
}
