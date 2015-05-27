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

cecp.c
Chess Engine Communication Protocol (CECP) module. Contains Input/Output
functions to talk with Chess Interfaces, like "xboard" and "glChess".
Runs in a separate thread, named internally as 'CECP Thread'.
Shares data with Search Thread (search.c).

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "cecp.h"
#include "history.h"
#include "eco.h"

/* CECP Thread main function */
void *cecp_loop(void *arg) {
    bool end_loop = FALSE;
    char command[MAX_SIZE_BUFFER];

    while(!end_loop) {
        /* Read commands from stdin and interpret them accordingly */
        fgets(command, MAX_SIZE_BUFFER, stdin);
        end_loop = cecp_command(ltrim(command));
    }

    return NULL;
}

/* Remove white spaces from the beggining of a string */
char *ltrim(char *p) {
    while(p && (*p == ' ' || *p == '\t' || *p == '\v' || *p == '\f'))
        p++;
    return p;
}

/* CECP Command parser */
bool cecp_command(char *c) {
    /* Cut off the newline char */
    if(c[strlen(c)-1] == '\n')
        c[strlen(c)-1] = '\0';

    /* Select command */
    if(!strncmp(c, "xboard", 6)) {
        answer_xboard();
    } else if(!strncmp(c, "protover ", 9)) {
        answer_protover(c);
    } else if(!strncmp(c, "accepted", 8)) {
        answer_accepted();
    } else if(!strncmp(c, "rejected", 8)) {
        answer_rejected();
    } else if(!strncmp(c, "new", 3)) {
        answer_new();
    } else if(!strncmp(c, "variant ", 8)) {
        answer_variant();
    } else if(!strncmp(c, "quit", 4)) {
        answer_quit();
        return TRUE;
    } else if(!strncmp(c, "random", 6)) {
        answer_random();
    } else if(!strncmp(c, "force", 5)) {
        answer_force();
    } else if(!strncmp(c, "go", 2)) {
        answer_go();
    } else if(!strncmp(c, "playother", 9)) {
        answer_playother();
    } else if(!strncmp(c, "white", 5)) {
        answer_white();
    } else if(!strncmp(c, "black", 5)) {
        answer_black();
    } else if(!strncmp(c, "level ", 6)) {
        answer_level(&c[6]);
    } else if(!strncmp(c, "st ", 3)) {
        answer_st(c);
    } else if(!strncmp(c, "sd ", 3)) {
        answer_sd(c);
    } else if(!strncmp(c, "time ", 5)) {
        answer_time();
    } else if(!strncmp(c, "otim ", 5)) {
        answer_otim();
    } else if(!strncmp(c, "usermove ", 9)) {
        answer_usermove(&c[9]);
    } else if(!strncmp(c, "?", 1)) {
        answer_interrogation();
    } else if(!strncmp(c, "ping ", 5)) {
        answer_ping(c);
    } else if(!strncmp(c, "draw", 4)) {
        answer_draw();
    } else if(!strncmp(c, "result ", 7)) {
        answer_result();
    } else if(!strncmp(c, "setboard ", 9)) {
        answer_setboard(&c[9]);
    } else if(!strncmp(c, "edit", 4)) {
        answer_edit();
    } else if(!strncmp(c, "hint", 4)) {
        answer_hint();
    } else if(!strncmp(c, "bk", 2)) {
        answer_bk();
    } else if(!strncmp(c, "undo", 4)) {
        answer_undo();
    } else if(!strncmp(c, "remove", 6)) {
        answer_remove();
    } else if(!strncmp(c, "hard", 4)) {
        answer_hard();
    } else if(!strncmp(c, "easy", 4)) {
        answer_easy();
    } else if(!strncmp(c, "post", 4)) {
        answer_post();
    } else if(!strncmp(c, "nopost", 6)) {
        answer_nopost();
    } else if(!strncmp(c, "analyze", 7)) {
        answer_analyze();
    } else if(!strncmp(c, "name ", 5)) {
        answer_name();
    } else if(!strncmp(c, "rating ", 7)) {
        answer_rating();
    } else if(!strncmp(c, "ics ", 4)) {
        answer_ics();
    } else if(!strncmp(c, "computer", 8)) {
        answer_computer();
    } else if(!strncmp(c, "pause", 5)) {
        answer_pause();
    } else if(!strncmp(c, "resume", 6)) {
        answer_resume();
    } else if(!strcmp(c, "")) {
        /* Ignore empty commands */
    } else {
        printf("Error (unknown command): %s\n", c);
    }

    return FALSE;
}

/* The 'xboard' command is the first one received by the engine.
 * If the engine has some kind of output (i. e. prompt), it must turn it off. */
void answer_xboard(void) {
    /* nop */
}

/* Protocol v2 signaling command. Tells the engine to output its configured
 * features to the chess interface. */
void answer_protover(char *c) {
    int tmp = 0;
    char **f, *features[] = {"ping=1", "setboard=1" , "playother=1", "san=0",
    "usermove=1", "time=0", "draw=1", "sigint=0", "sigterm=0","reuse=1",
    "analyze=0", "myname=\"ChessD BoT\"", "variants=\"normal\"", "colors=0",
    "ics=0", "name=1", "pause=0", "done=1", NULL};

    /* If its not version 2, do nothing */
    if(!(sscanf(c, "protover %d", &tmp) == 1 && tmp == 2))
        return;

    /* Send the features*/
    for(f = features; *f; f++)
        printf("feature %s\n", *f);
}

/* The 'accepted' command is received when the chess interface agrees with
 * a given feature option. */
void answer_accepted(void) {
    /* nop */
}

/* The 'rejected' command is received when the chess interface do not agree
 * with a given feature option. In that case, the engine must quit. */
void answer_rejected(void) {
    quit("Error: Rejected feature!\n");
}

/* The 'new' command resets the board to the standard chess starting position.
 * Also sets White on move, leaves force mode and sets the engine to
 * play Black. */
void answer_new(void) {
    pthread_mutex_lock(&mutex);
    init_history();
    clear_board(board);
    board = set_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    if(board == NULL)
        quit("Error: Could not setup new board!\n");
    pthread_mutex_unlock(&mutex);
    set_status(NOP);
}

/* Sets the variant chess style to be played. */
void answer_variant(void) {
    /* Currently only "standard" is supported */
}

/* Upon receiving the 'quit' command, the engine should immediately exit */
void answer_quit(void) {
    /* Tells the search thread to quit */
    set_status(QUIT);
}

/* The command toggles "random" mode. In random mode, the engine adds a small
 * random value to its evaluation function to vary its play. The 'new' command
 * sets random mode off. */
void answer_random(void) {
    /* Not used */
}

/* Set the engine to play neither color ("force" mode). The engine should check
 * that moves received in force mode are legal and made in the proper turn,
 * but should not think, ponder, or make moves of its own. */
void answer_force(void) {
    /* Tells the search thread to enter "force" mode*/
    set_status(FORCE);
}

/* Leave force mode and set the engine to play the color that is on move.
 * Start thinking and eventually make a move. */
void answer_go(void) {
    char out[MAX_SIZE_BUFFER];
    move_t *m;

    /* Set search thread to search mode and wait */
    set_status_and_wait(SEARCH);

    /* Get the move just played by us */
    m = peek_history_move_top();

    /* Translate to string and tell our move to the opponent */
    if(!move_to_coord(out, m))
        quit("Error: Bad move!\n");
    printf("move %s\n", out);

    /* Check end game conditions */
    check_game_over();
}


/* Leave force mode and set the engine to play the color that is not on move.
 * If the engine later receives a move, it should start thinking and eventually
 * reply. */
void answer_playother(void) {
    /* Tells the search thread to do anything, waiting for a user's move */
    set_status(NOP);
}

/* This command is obsolete as of protocol version 2. Set White on move.
 * Set the engine to play Black. */
void answer_white(void) {
    /* Not used */
}

/* This command is obsolete as of protocol version 2. Set Black on move.
 * Set the engine to play White. */
void answer_black(void) {
    /* Not used */
}

/* Sets the Time control. Arguments are: Moves per time control, time control
 * period, and increment. The meaning of such terms can be better clarified by
 * visiting http://www.tim-mann.org/xboard/engine-intf.html */
void answer_level(char *c) {
    char *tok1 = NULL, *tok2 = NULL, *save1 = NULL, *save2 = NULL;
    uint32_t moves = 40, base = 300, inc = 0;

    /* Get Moves per time control */
    tok1 = strtok_r(c, " ", &save1);
    if(tok1)
        moves = atoi(tok1);
    else
        return;

    /* In order to avoid division by zero, set moves value to default */
    if(moves == 0)
        moves = 40;

    /* Get minutes [and seconds] in each tim control */
    tok1 = strtok_r(NULL, " ", &save1);
    tok2 = strtok_r(tok1, ":", &save2);
    if(tok2) {
        base = atoi(tok2) * 60;
        tok2 = strtok_r(NULL, ":", &save2);
    if(tok2)
        base += atoi(tok2);
    } else if(tok1) {
        base = atoi(tok1) * 60;
    } else {
        return;
    }

    /* Get Increment */
    tok1 = strtok_r(NULL, " ", &save1);
    if(tok1)
        inc = atoi(tok1);
    else
        return;

    /* Configure alarm settings */
    if(((base / moves) + inc) < get_config_alarm())
        config_alarm((base / moves) + inc);
}

/* Set an exact number of seconds per move. */
void answer_st(char *c) {
    int n;
    if(sscanf(c, "st %d", &n) == 1 && n > 0)
        config_alarm(n);
}

/* The engine should limit its thinking to the given number of ply. */
void answer_sd(char *c) {
    int n;
    if(sscanf(c, "sd %d", &n) == 1 && n >= 2)
        max_depth = n;
}

/* Set a clock that always belongs to the engine.
 * This command is disabled by the engine via the 'feature' command. */
void answer_time(void) {
    /* Not used */
}

/* Set a clock that always belongs to the opponent.
 * This command is disabled by the engine via the 'feature' command. */
void answer_otim(void) {
    /* Not used */
}

/* Receives a user's move. If the move is illegal, print an error message.
 * If the move is legal and in turn, make it. If not in force mode, start
 * thinking, and eventually make a move.*/
void answer_usermove(char *c) {
    static char out[MAX_SIZE_BUFFER];
    static move_t *m, userm;

    /* If the user's move is invalid, return */
    if(!coord_to_move(c, &userm)) {
        printf("Illegal move: %s\n", c);
        return;
    }

    /* Check if the movement is valid or not */
    if(!check_valid_move(board, userm)) {
        printf("Illegal move: %s\n", c);
        return;
    }

    /* Perform the move if its valid */
    move(board, userm);

    /* Check end game conditions */
    if(!check_game_over() && get_status() != FORCE) {
        /* Set search thread to search mode and wait */
        set_status_and_wait(SEARCH);

        /* Get the move just played by us */
        m = peek_history_move_top();

        /* Translate to string and tell our move to the opponent */
        if(!move_to_coord(out, m))
            quit("Error: Bad move!\n");
        printf("move %s\n", out);

        /* Check end game conditions */
        check_game_over();
    }
}

/* Stops search process at once */
void answer_interrogation(void) {
    set_timeout(TRUE);
}

/* When the engine recieves this command, it must reply with a 'pong' and the
 * same (integer) argument. */
void answer_ping(char *c) {
    int tmp;
    if(sscanf(c, "ping %d", &tmp) == 1)
        printf("pong %d\n", tmp);
}

/* The engine's opponent offers the engine a draw. */
void answer_draw(void) {
    /* The Engine accepts the draw offer, if it considers the position a draw */
    if(evaluate_draw(board))
        printf("offer draw\n");
}

/* Receives the score and the reason for the game to be finished. */
void answer_result(void) {
    /* nop */
}

/* The 'setboard' command is the new way to set up positions, beginning in
 * protocol version 2. The argument of this command is a position in
 * Forsythe-Edwards Notation (FEN), as defined in the PGN standard. */
void answer_setboard(char *c) {
    pthread_mutex_lock(&mutex);
    init_history();
    clear_board(board);
    board = set_board(c);
    if(board == NULL)
        quit("Error: Could not setup new board!\n");
    /* We cannot use ECO, since we don't have the full history log */
    can_use_eco = FALSE;
    pthread_mutex_unlock(&mutex);
    set_status(NOP);
}

/* The 'edit' command is the old way to set up positions.
 * Use 'setboard' instead. */
void answer_edit(void) {
    /* Not used */
}

/* The opponent has asked for a move hint. This engine does not give hints. */
void answer_hint(void) {
    /* nop */
}


/* Show book moves from this position, if any. Not implemented. */
void answer_bk(void) {
    /* Not used */
}

/* The opponent asks the engine to backup one (half-)move. */
void answer_undo(void) {
    unmove(board);
}

/* The opponent asks the engine to backup one full-move (two ply). */
void answer_remove(void) {
    unmove(board);
    unmove(board);
}

/* Turn on pondering (thinking on the opponent's time, also known as
 * "permanent brain"). Pondering not implemented */
void answer_hard(void) {
    /* nop */
}

/* Turn off pondering. Pondering not implemented */
void answer_easy(void) {
    /* nop */
}

/* Turn on thinking/pondering output. Pondering not implemented */
void answer_post(void) {
    /* nop */
}

/* Turn off thinking/pondering output. Pondering not implemented */
void answer_nopost(void) {
}

/* Enter "analyze" mode. This engine does not support "analyze" mode. */
void answer_analyze(void) {
    /* nop */
}

/* This command informs the engine of its opponent's name. */
void answer_name(void) {
    /* nop */
}

/* This command informs the engine of its opponent's rating. */
void answer_rating(void) {
    /* nop */
}

/* This command informs the engine of the ICS address, if any. This command is
 * disabled by the engine, via 'feature' command. */
void answer_ics(void) {
    /* nop */
}

/* This informs the engine that its opponent is also an engine. */
void answer_computer(void) {
    /* nop */
}

/* This command is disabled by the engine via the 'feature' command.
 * The "pause" command puts the engine into a special state where it does not
 * think, ponder, or otherwise consume significant CPU time. The current
 * thinking or pondering (if any) is suspended and both player's clocks are
 * stopped. The only command that the interface may send to the engine while it
 * is in the paused state is "resume". The paused thinking or pondering (if any)
 * resumes from exactly where it left off, and the clock of the player on move
 * resumes running from where it stopped. */
void answer_pause(void) {
    /* Not used */
}

void answer_resume(void) {
    /* Not used */
}

/* Check end game conditions and output accordingly */
bool check_game_over(void) {
    switch(end(board)) {
    case CHECK_MATE:
        if(board->onmove == COLOR_WHITE)
            printf("0-1 {Black has won by checkmate}\n");
        else
            printf("1-0 {White has won by checkmate}\n");
        break;
    case STALE_MATE:
        printf("1/2-1/2 {Stalemate}\n");
        break;
    case REPETITION:
        printf("1/2-1/2 {Draw by three fold repetition rule}\n");
        break;
    case FIFTY_MOVES:
        printf("1/2-1/2 {Draw by 50 movements rule}\n");
        break;
    case TWO_KINGS:
        printf("1/2-1/2 {Draw by lack of material}\n");
        break;
    case NO_MATE:
    default:
        return FALSE;
    }

    return TRUE;
}
