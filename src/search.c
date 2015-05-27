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

search.c
Search Thread module. This is the core of the Chess Engine. It contains
functions to perform a heuristic search algorithm (Minimax with Alpha-Beta
pruning) looking for the next best move to make at a specific chess board state,
in an Iterative Deepening way. It also has functions and structures to
constraint this search process within a time limit, previously chosen by the
user. Basically it recieves commands from the CECP Thread, and after performing
the search, returns an answer (a chess movement).

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "search.h"
#include "heuristic.h"
#include "transposition.h"
#include "eco.h"
#include "history.h"
#include "levels.h"

/* Main board structure, shared with CECP Thread */
board_t *board;
/* Search state */
static status_t status = NOP;
/* Timeout flag to control spent time in the search */
static bool timeout = FALSE;
/* Mutex to control access to timeout flag */
static pthread_mutex_t timeout_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timeval alarm_clock = {ALARM_INIT_SEC, ALARM_INIT_USEC};

/* Used to hold whose color is on move during a search process */
static uint8_t onmove;

/* Maximum depth of the Minimax search tree */
uint8_t max_depth;

/* Search thread main function */
void *search_loop(void *arg) {
    uint8_t ply;
    move_t mv, mv_tmp;

    SET_BLANK_MOVE(mv);
    SET_BLANK_MOVE(mv_tmp);

    pthread_mutex_lock(&mutex);

    /* Initializations */
    precompute_moves();
    precompute_distances();
    init_zobrist_keys();
    init_history();
    init_transposition_table();
    config_alarm(config->max_seconds);
    max_depth = config->max_depth;

    /* Setup board */
    board = set_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    if(board == NULL)
    	quit("Error: Could not setup new board!\n");

    /* ECO = encyclopedia of chess openings */

    if(atoi(config->name) >= 50)
        if(!load_eco(board))
            quit("Error: Could not load Encyclopedia of Chess Openings!\n");

    pthread_mutex_unlock(&mutex);
 
    /* Keep alive until the status changes to QUIT */
    while(status != QUIT) {
    	switch(status) {
    	case NOP:
    	    /* NOP: just wait for a change in status */
    	    pthread_mutex_lock(&mutex);
    	    while(status == NOP)
    	    	pthread_cond_wait(&cond, &mutex);
    	    pthread_mutex_unlock(&mutex);
    	    break;
    	case FORCE:
    	    /* FORCE: wait for a change in status but don't start searches */
    	    pthread_mutex_lock(&mutex);
    	    while(status == FORCE)
    	    	pthread_cond_wait(&cond, &mutex);
    	    pthread_mutex_unlock(&mutex);
    	    break;
    	case SEARCH:
    	    /* Iterative Deepening Search */
    	    /* Save the on-move color */
    	    onmove = board->onmove;
    	    /* Sets as blank the move to be played*/
    	    SET_BLANK_MOVE(mv);
    	    /* Starts counting the time */
    	    start_alarm();
    	    /* For each depth, search with alpha-beta minimax */
    	    for(ply = 2; ply <= max_depth; ply += 2) {
    	    	mv_tmp = alpha_beta(board, -MAX_HEU, MAX_HEU, ply);
    	    	/* Did we run out of time? If so, stops deepening iterations */
    	    	if(get_timeout())
    	    	    break;
    	    	mv = mv_tmp;
    	    }
    	    /* Stops counting the time, if it hasn't already reached limit */
    	    stop_alarm();
    	    /* If the move is still blank, use the partial move found */
    	    if(IS_BLANK_MOVE(mv))
    	    	mv = mv_tmp;
    	    /* Perform the move found in the search */
    	    move(board, mv);
    	    /* Returns to the NOP status */
    	    set_status(NOP);
    	    break;
    	case PONDER:
    	    /* Reserved for future use */
    	    set_status(NOP);
    	    break;
    	default:
    	    quit("Error: Invalid search status!\n");
    	}
    }
    
    /* Clean up memory */
    clear_eco();
    clear_transposition_table();
    clear_history();
    clear_board(board);

    /* Exit Search Thread */
    return NULL;
}

/* Alpha Beta Pruning - Minimax Search Algorithm */
move_t alpha_beta(board_t *b, int32_t alpha, int32_t beta, uint32_t ply) {
    uint8_t i, type;
    move_list_t *list;
    move_t m, best;

    /* Query ECO tree */
    if(atoi(config->name) >= 50)
        if(query_eco(&m))	    
    	    return m;

    /* Query transposition table */
    type = query_transposition(b->hash, alpha, beta, ply, &m);
    switch(type) {
    case TYPE_ALPHA:
    	/* Chooses the best alpha between the old and the one from the table */
        alpha = MAX(alpha, m.eval);
    	/* If we have a Beta cutoff, returns */
    	if(alpha >= beta)
    	    return m;
    	break;
    case TYPE_BETA:
    	/* Chooses the best beta between the old and the one from the table */
    	beta = MIN(beta, m.eval);
    	/* If we have a Beta cutoff, returns */
    	if(alpha >= beta)
    	    return m;
    	break;
    case TYPE_EXACT:
    	/* If the table had an exact evaluation, returns it */
    	return m;
    case TYPE_INVALID:
    	/* If the transposition is invalid, just ignore and keep going*/
    	break;
    }

    /* If it's a leaf node, evaluate it properly */
    if(ply == 0) {
    	m.eval = heuristic(b, onmove);
    	return m;
    }

    /* Default type of the value to be inserted in the Transposition table */
    type = TYPE_ALPHA;

    /* Initialize the best possible move as blank */
    SET_BLANK_MOVE(best);

    /* Get the possible next moves */
    list = gen_move_list(b, FALSE);
    /* For each possible next move... */
    for(i = 0; i < list->size; i++) {
    	/* Let's see the board after that move... */
    	move(b, list->move[i]);

    	/* Did we reach any end game condition? */
    	switch(end(b)) {
    	case CHECK_MATE:
    	    m.eval = (onmove == b->onmove) ? -MAX_HEU : MAX_HEU;
    	    break;
    	case STALE_MATE:
    	case REPETITION:
    	case FIFTY_MOVES:
    	case TWO_KINGS:
    	    m.eval = -MAX_HEU;
    	    break;
    	case NO_MATE:
    	default:
    	    /* If not, keep searching down in the search tree */
    	    m = alpha_beta(b, -beta, -alpha, ply - 1);
    	    m.eval = -m.eval;
    	    break;
    	}

    	/* Restores the previous board (before the possible move) */
    	unmove(b);

    	/* Beta cutoff */
    	if(m.eval >= beta) {
    	    best = list->move[i];
    	    best.eval = m.eval;
    	    type = TYPE_BETA;
    	    break;
    	/* Alpha cutoff */
    	} else if(m.eval > alpha) {
    	    best = list->move[i];
    	    alpha = best.eval = m.eval;
    	    type = TYPE_EXACT;
    	/* Best possible move until now */
    	} else if(i == 0 || m.eval > best.eval) {
    	    best = list->move[i];
    	    best.eval = m.eval;
    	}

    	/* If our time's up, return immediately */
    	if(get_timeout()) {
    	    clear_move_list(list);
    	    return best; /*break;*/
    	}
    }

    /* Update the Transposition table */
    add_transposition(b->hash, type, ply, best);

    /* Clear temporary information and return */
    clear_move_list(list);
    return best;
}

/* Quiescence Search */
move_t quiescence(board_t *b, int32_t alpha, int32_t beta) {
    uint8_t i;
    move_t m, best;
    move_list_t *list;
    
    /* Initialize the best possible move as blank */
    SET_BLANK_MOVE(best);

    m.eval = heuristic(b, onmove);
    if(m.eval >= beta) {
    	best.eval = m.eval;
    	return best;
    } else if(m.eval > alpha) {
    	alpha = m.eval;
    }

    /* Get the next possible Good captures only */
    list = gen_move_list(b, TRUE);

    /*TODO: Re-order the move list (SEE - Static Exchange Eval) */
    /*reorder_move_list(b, list); */

    /* For each possible next move... */
    for(i = 0; i < list->size; i++) {
    	/* Let's see the board after that move... */
    	move(b, list->move[i]);

    	/* Quiescence Search recursion */
    	m = quiescence(b, -beta, -alpha);
    	m.eval = -m.eval;

    	/* Restores the previous board (before the possible move) */
    	unmove(b);

    	/* Beta cutoff */
    	if(m.eval >= beta) {
    	    best = list->move[i];
    	    best.eval = m.eval;
    	    break;
    	/* Alpha cutoff */
    	} else if(m.eval > alpha) {
    	    best = list->move[i];
    	    alpha = best.eval = m.eval;
    	/* Best possible move until now */
    	} else if(i == 0 || m.eval > best.eval) {
    	    best = list->move[i];
    	    best.eval = m.eval;
    	}
    }

    /* Clear temporary information and return */
    clear_move_list(list);
    return best;
}

/* Set search thread status and signalize the global condition */
void set_status(status_t s) {
    pthread_mutex_lock(&mutex);
    status = s;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

/* Returns the status of the search process */
status_t get_status(void) {
    return status;
}

/* Set search thread status, signalize the global condition
 * and then wait until the global condition is signalized back. */
void set_status_and_wait(status_t s) {
    pthread_mutex_lock(&mutex);
    status = s;
    pthread_cond_signal(&cond);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

/* Return the value of the timeout flag */
bool get_timeout(void) {
    bool t;
    pthread_mutex_lock(&timeout_mutex);
    t = timeout;
    pthread_mutex_unlock(&timeout_mutex);
    return t;
}

/* Set the timeout flag */
void set_timeout(bool t) {
    pthread_mutex_lock(&timeout_mutex);
    timeout = t;
    pthread_mutex_unlock(&timeout_mutex);
}

/* Bind the signal handler function to the SIGALRM signal;
 * Also set the intial value of the alarm clock */
void config_alarm(uint32_t secs) {
    signal(SIGALRM, sigalrm_handler);
    alarm_clock.tv_sec = secs;
    alarm_clock.tv_usec = 0;
}

/* Returns the configured alarm time at the moment */
uint32_t get_config_alarm(void) {
    return (uint32_t) alarm_clock.tv_sec;
}

/* Start ticking the alarm */
void start_alarm(void) {
    struct itimerval t;
    t.it_interval.tv_sec = t.it_interval.tv_usec = 0;
    t.it_value = alarm_clock;
    if(setitimer(ITIMER_REAL, &t, NULL) == -1)
        quit("Error: Could not start alarm!\n");
}

/* Stop the alarm clock.
 * Returns the time left for the alarm to sound */
struct timeval stop_alarm(void) {
    struct itimerval old, t;

    /* Reset the timeout */
    set_timeout(FALSE);

    /* Get the time left */
    if(getitimer(ITIMER_REAL, &old) == -1)
        quit("Error: Could not get timer!\n");

    /* Reset the alarm clock to zero */
    t.it_interval.tv_sec = t.it_interval.tv_usec = 0;
    t.it_value.tv_sec = t.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &t, NULL) == -1)
        quit("Error: Could not set timer!\n");
    
    return old.it_value;
}

/* SIGALRM signal handler. 
 * It's call means that we've run out of time in the search process */
void sigalrm_handler(int useless) {
    set_timeout(TRUE);
}
