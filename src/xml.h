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

xml.h
XML Parser Header. Contains the defition of a structure for representing XML
elements. Also has some function prototypes for manipulating such structures.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _XML_H_
#define _XML_H_

#include "main.h"
#include <expat.h>

#define XML_BUFF_SIZE 4096

/* Definition of a XML node (element). */
typedef struct _xml_node xml_node_t;
struct _xml_node {
	char *name;
	int attributes;
	char **attribute_keys;
	char **attribute_values;
	int nodes;
	xml_node_t **child_nodes;
	char *data;
	xml_node_t *father;
};

xml_node_t *xml_parser(char *filename);
void clean_xml_node(xml_node_t *n);
xml_node_t **get_elements_by_tag_name(xml_node_t *n, char *tag_name);
char *get_attribute(xml_node_t *n, char *key);

#endif
