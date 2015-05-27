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

cecp.h
Chess Engine Communication Protocol (CECP) module header file. Contains some
function prototypes to be used inside the CECP module, and also some constants.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _CECP_H_
#define _CECP_H_

#include "main.h"
#include "board.h"
#include "moves.h"
#include "search.h"

#define MAX_SIZE_BUFFER 256

void *cecp_loop(void *arg);
char *ltrim(char *);
bool cecp_command(char *c);
void answer_xboard(void);
void answer_protover(char *c);
void answer_accepted(void);
void answer_rejected(void);
void answer_new(void);
void answer_variant(void);
void answer_quit(void);
void answer_random(void);
void answer_force(void);
void answer_go(void);
void answer_playother(void);
void answer_white(void);
void answer_black(void);
void answer_level(char *c);
void answer_st(char *c);
void answer_sd(char *c);
void answer_time(void);
void answer_otim(void);
void answer_usermove(char *c);
void answer_interrogation(void);
void answer_ping(char *c);
void answer_draw(void);
void answer_result(void);
void answer_setboard(char *c);
void answer_edit(void);
void answer_hint(void);
void answer_bk(void);
void answer_undo(void);
void answer_remove(void);
void answer_hard(void);
void answer_easy(void);
void answer_post(void);
void answer_nopost(void);
void answer_analyze(void);
void answer_name(void);
void answer_rating(void);
void answer_ics(void);
void answer_computer(void);
void answer_pause(void);
void answer_resume(void);
bool check_game_over(void);

#endif
