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

levels.c
Level selection module. Responsible for loading and selecting a particular
difficulty level.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "levels.h"
#include "math.h"

/* The list of al loaded levels' configuration. */
static level_t **levels;
static int num_levels;
/* The Global selected level configuration. */
level_t *config;

/* Loads the levels described in the levels' configuratino file
 * If any sintax error is found, the program is aborted.
 */
void load_levels() {
    xml_node_t *root, **level, **aux;
    char *attr;
    int i;

    root = xml_parser(LEVELS_CONFIG_FILE);
    if(root->child_nodes == NULL || strcmp(root->child_nodes[0]->name, "chessdbot"))
        quit("Error: Malformed levels configuration file!\n");

    level = get_elements_by_tag_name(root, "level");
    if(level == NULL)
        quit("Error: Malformed levels configuration file!\n");

    for(num_levels = 0, aux = level; *aux; aux++, num_levels++);

    levels = (level_t **) malloc(num_levels * sizeof(level_t *));
    if(levels == NULL)
        quit("Error: Could not load levels configuration file!\n");

    for(i = 0; i < num_levels; i++) {
        levels[i] = (level_t *) malloc(sizeof(level_t));
        if(levels[i] == NULL)
            quit("Error: Could not load levels configuration file!\n");

        attr = get_attribute(level[i], "name");
        levels[i]->name = malloc((strlen(attr)+1) * sizeof(char));
        if(levels[i]->name == NULL)
            quit("Error: Could not load levels configuration file!\n");
        strcpy(levels[i]->name, attr);
        aux = get_elements_by_tag_name(level[i], "search");
        if(aux == NULL)
            quit("Error: Malformed levels configuration file!\n");
        attr = get_attribute(*aux, "max_depth");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->max_depth = atoi(attr);
        if(levels[i]->max_depth < 2)
            quit("Error: max_depth must be at least 2\n");
        attr = get_attribute(*aux, "max_seconds");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->max_seconds = atoi(attr);
        if(levels[i]->max_seconds < 1)
            quit("Error: max_seconds must be at least 1\n");
        free(aux);

        aux = get_elements_by_tag_name(level[i], "heuristic");
        if(aux == NULL)
            quit("Error: Malformed levels configuration file!\n");

        attr = get_attribute(*aux, "pawn_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->pawn_val = atoi(attr);
        if(levels[i]->pawn_val < 0)
            quit("Error: pawn_val must be at least 0\n");
        attr = get_attribute(*aux, "bishop_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bishop_val = atoi(attr);
        if(levels[i]->bishop_val < 0)
            quit("Error: bishop_val must be at least 0\n");
        attr = get_attribute(*aux, "knight_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->knight_val = atoi(attr);
        if(levels[i]->knight_val < 0)
            quit("Error: knight_val must be at least 0\n");
        attr = get_attribute(*aux, "rook_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->rook_val = atoi(attr);
        if(levels[i]->rook_val < 0)
            quit("Error: rook_val must be at least 0\n");
        attr = get_attribute(*aux, "queen_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->queen_val = atoi(attr);
        if(levels[i]->queen_val < 0)
            quit("Error: queen_val must be at least 0\n");
        attr = get_attribute(*aux, "king_val");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->king_val = atoi(attr);
        if(levels[i]->king_val < 0)
            quit("Error: king_val must be at least 0\n");
        attr = get_attribute(*aux, "factor_material");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_material = atoi(attr);
        if(levels[i]->factor_material < 0)
            quit("Error: factor_material must be at least 0\n");
        attr = get_attribute(*aux, "factor_development");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_development = atoi(attr);
        if(levels[i]->factor_development < 0)
            quit("Error: factor_development must be at least 0\n");
        attr = get_attribute(*aux, "factor_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_pawn = atoi(attr);
        if(levels[i]->factor_pawn < 0)
            quit("Error: factor_pawn must be at least 0\n");
        attr = get_attribute(*aux, "factor_bishop");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_bishop = atoi(attr);
        if(levels[i]->factor_bishop < 0)
            quit("Error: factor_bishop must be at least 0\n");
        attr = get_attribute(*aux, "factor_king");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_king = atoi(attr);
        if(levels[i]->factor_king < 0)
            quit("Error: factor_king must be at least 0\n");
        attr = get_attribute(*aux, "factor_knight");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        if(levels[i]->factor_knight < 0)
            quit("Error: factor_knight must be at least 0\n");
        levels[i]->factor_knight = atoi(attr);
        attr = get_attribute(*aux, "factor_queen");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_queen = atoi(attr);
        if(levels[i]->factor_queen < 0)
            quit("Error: factor_queen must be at least 0\n");
        attr = get_attribute(*aux, "factor_rook");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->factor_rook = atoi(attr);
        if(levels[i]->factor_rook < 0)
            quit("Error: factor_rook must be at least 0\n");
        attr = get_attribute(*aux, "bonus_early_queen_move");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_early_queen_move = atoi(attr);
        attr = get_attribute(*aux, "bonus_early_bishop_stuck");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_early_bishop_stuck = atoi(attr);
        attr = get_attribute(*aux, "bonus_early_knight_stuck");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_early_knight_stuck = atoi(attr);
        attr = get_attribute(*aux, "bonus_has_castled");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_has_castled = atoi(attr);
        attr = get_attribute(*aux, "bonus_hasnt_castled");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_hasnt_castled = atoi(attr);
        attr = get_attribute(*aux, "bonus_passed_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_passed_pawn = atoi(attr);
        attr = get_attribute(*aux, "bonus_isolated_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_isolated_pawn = atoi(attr);
        attr = get_attribute(*aux, "bonus_backward_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_backward_pawn = atoi(attr);
        attr = get_attribute(*aux, "bonus_doubled_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_doubled_pawn = atoi(attr);
        attr = get_attribute(*aux, "bonus_tripled_pawn");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_tripled_pawn = atoi(attr);
        attr = get_attribute(*aux, "bonus_doubled_bishop");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_doubled_bishop = atoi(attr);
        attr = get_attribute(*aux, "bonus_fianchetto_bishop");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_fianchetto_bishop = atoi(attr);
        attr = get_attribute(*aux, "bonus_knight_on_edge");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_knight_on_edge = atoi(attr);
        attr = get_attribute(*aux, "bonus_knight_on_hole");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_knight_on_hole = atoi(attr);
        attr = get_attribute(*aux, "bonus_rook_open_file");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_rook_open_file = atoi(attr);
        attr = get_attribute(*aux, "bonus_rook_halfopen_file");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_rook_halfopen_file = atoi(attr);
        attr = get_attribute(*aux, "bonus_queen_open_file");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_queen_open_file = atoi(attr);
        attr = get_attribute(*aux, "bonus_queen_halfopen_file");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_queen_halfopen_file = atoi(attr);
        attr = get_attribute(*aux, "bonus_center_control");
        if(attr == NULL)
            quit("Error: Malformed levels configuration file!\n");
        levels[i]->bonus_center_control = atoi(attr);
        free(aux);
    }
    free(level);
    clean_xml_node(root);
}

void adjust_level(char *lvl){
    int factor;

    factor = atoi(lvl);
    config->max_depth = 2 + (factor/4);
	printf("%d\n",config->max_depth);
    config->max_seconds = 2 + ((factor-9)/8);
	printf("%d\n",config->max_seconds);

    config->pawn_val = (factor+9)*-2/11+20;
	printf("%d\n",config->pawn_val);
    config->bishop_val = (factor-3)/-10+12;
	printf("%d\n",config->bishop_val);
    config->knight_val =  (factor-2)/-10+12;
	printf("%d\n",config->knight_val);
    config->rook_val = (factor-4)*-3/100+7;
	printf("%d\n",config->rook_val);
    config->queen_val = (factor-5)/25+6;
	printf("%d\n",config->queen_val);
    config->king_val = factor;
	printf("%d\n",config->king_val);

    config->factor_material = (factor-22)*13/10;
    if (factor < 24)
	config->factor_material = 1;
    if (factor < 12)
        config->factor_material = 0;
	printf("%d\n",config->factor_material);
    config->factor_development = (factor-6)/5+1;
	printf("%d\n",config->factor_development);

    config->factor_pawn = (factor-5)/-10+10;
	printf("%d\n",config->factor_pawn);
    config->factor_bishop = (factor-6)*2/-25+8;
	printf("%d\n",config->factor_bishop);
    config->factor_knight = (factor-7)*2/-25+8;
	printf("%d\n",config->factor_knight);
    config->factor_queen = (factor-8)/-25+4;
	printf("%d\n",config->factor_queen);
    config->factor_rook = (factor)*-7/100+8;
	printf("%d\n",config->factor_rook);
    config->factor_king = (factor-9)*-11/100+11;
	printf("%d\n",config->factor_king);

    config->bonus_early_queen_move = (factor-1)/-5;
    config->bonus_early_bishop_stuck = (factor-2)/-4+5;
    config->bonus_early_knight_stuck = (factor-3)/-4+5;
    config->bonus_has_castled = (factor-4)/4-4;
    config->bonus_hasnt_castled = (factor)/-4+5;
    config->bonus_passed_pawn = (factor-1)/4+5;
    config->bonus_isolated_pawn = (factor-2)/-4+5;
    config->bonus_backward_pawn = (factor-3)/-4+5;
    config->bonus_doubled_pawn = (factor-4)/-4+4;
    config->bonus_tripled_pawn = (factor)/-5;
    config->bonus_doubled_bishop = (factor-1)/4-5;
    config->bonus_fianchetto_bishop = (factor-2)/4-5;
    config->bonus_knight_on_edge = (factor-3)/-4+4;
    config->bonus_knight_on_hole = (factor-4)/4-4;
    config->bonus_rook_open_file = (factor)/5;
    config->bonus_rook_halfopen_file = (factor-1)/10;
    config->bonus_queen_open_file = (factor-2)/5;
    config->bonus_queen_halfopen_file = (factor-3)/10;
    config->bonus_center_control = (factor-4)*7/50+10;
}

/* Selects a particular difficulty level, among those previously loaded. */
void validate_level(char *lvl) {
    int aux;    
    
    aux = atoi(lvl);
    if (aux > 0 && aux <= 100)
        printf("Selected level %d.\n", aux);
    else
        quit("Error: Invalid level chosen!!! Type a level between 1 and 100");
}

void select_level(char *lvl) {
    int i;
    config = levels[0];
    if(lvl != NULL) {
        for(i = 0; i < num_levels; i++) {
            if(!strcmp(lvl, levels[i]->name)) {
                config = levels[i];
                break;
            }
        }
    }
}


/* Cleans up all the memory used by the loaded levels. */
void clean_levels(void) {
    int i;
    for(i = 0; i < num_levels; i++) {
        free(levels[i]->name);
        free(levels[i]);
    }
    free(levels);
}
