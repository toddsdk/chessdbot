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

xml.c
XML Parser Module. Responsible for opening and parsing XML files and creating
and managing suitable structures for XML elements.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "xml.h"

/* Creates an empty XML node. */
xml_node_t *init_xml_node(void) {
	xml_node_t *n = (xml_node_t *) malloc(sizeof(xml_node_t));
	if(n == NULL)
		quit("Error: Could not create XML node!\n");
	n->name = NULL;
	n->attributes = 0;
	n->attribute_keys = NULL;
	n->attribute_values = NULL;
	n->nodes = 0;
	n->child_nodes = NULL;
	n->data = NULL;
	n->father = NULL;
	return n;
}

/* Cleans a XML node and all of its child nodes recursively. */
void clean_xml_node(xml_node_t *n) {
	int i;
	if(n == NULL)
		return;
	if(n->name != NULL)
		free(n->name);
	for(i = 0; i < n->attributes; i++) {
		free(n->attribute_keys[i]);
		free(n->attribute_values[i]);
	}
	if(n->attribute_keys != NULL)
		free(n->attribute_keys);
	if(n->attribute_values != NULL)
		free(n->attribute_values);
	for(i = 0; i < n->nodes; i++) {
		clean_xml_node(n->child_nodes[i]);
	}
	if(n->child_nodes != NULL)
		free(n->child_nodes);
	if(n->data != NULL)
		free(n->data);
	free(n);
}

/* Sets the name of a XML node. */
void set_name_xml_node(xml_node_t *n, char *name) {
	if(n->name != NULL)
		quit("Error: String previously allocated at XML parse!\n");
	n->name = (char *) malloc((strlen(name)+1) * sizeof(char));
	if(n->name == NULL)
		quit("Error: Could not create XML node!\n");
	strcpy(n->name, name);
}

/* Adds an attribute (key and value) to a XML node. */
void add_attr_xml_node(xml_node_t *n, char *key, char *value) {
	if(n->attributes == 0) {
		n->attribute_keys = (char **) malloc(sizeof(char *));
		if(n->attribute_keys == NULL)
			quit("Error: Could not create XML node!\n");
		n->attribute_values = (char **) malloc(sizeof(char *));
		if(n->attribute_values == NULL)
			quit("Error: Could not create XML node!\n");
	} else {
		n->attribute_keys = (char **) realloc(n->attribute_keys, (n->attributes+1) * sizeof(char *));
		if(n->attribute_keys == NULL)
			quit("Error: Could not create XML node!\n");
		n->attribute_values = (char **) realloc(n->attribute_values, (n->attributes+1) * sizeof(char *));
		if(n->attribute_values == NULL)
			quit("Error: Could not create XML node!\n");
	}
	n->attribute_keys[n->attributes] = (char *) malloc((strlen(key)+1) * sizeof(char));
	if(n->attribute_keys[n->attributes] == NULL)
		quit("Error: Could not create XML node!\n");
	n->attribute_values[n->attributes] = (char *) malloc((strlen(value)+1) * sizeof(char));
	if(n->attribute_values[n->attributes] == NULL)
		quit("Error: Could not create XML node!\n");
	strcpy(n->attribute_keys[n->attributes], key);
	strcpy(n->attribute_values[n->attributes], value);
	n->attributes++;
}

/* Sets the data of a XML node. */
void set_data_xml_node(xml_node_t *n, char *data, int len) {
	if(n->data != NULL)
		quit("Error: String previously allocated at XML parse!\n");
	n->data = (char *) malloc((len+1) * sizeof(char));
	if(n->data == NULL)
		quit("Error: Could not create XML node!\n");
	strncpy(n->data, data, len);
	n->data[len] = '\0';
}

/* Adds a child node to an existing XML node. */
void add_child_xml_node(xml_node_t *n, xml_node_t *child) {
	xml_node_t **old_nodes = NULL;
	if(n->nodes == 0) {
		n->child_nodes = (xml_node_t **) malloc(sizeof(xml_node_t *));
		if(n->child_nodes == NULL)
			quit("Error: Could not create XML node!\n");
		n->child_nodes[0] = child;
		n->child_nodes[0]->father = n;
		n->nodes = 1;
	} else {
		old_nodes = n->child_nodes;
		n->child_nodes = (xml_node_t **) malloc((n->nodes+1) * sizeof(xml_node_t *));
		if(n->child_nodes == NULL)
			quit("Error: Could not create XML node!\n");
		memcpy(n->child_nodes, old_nodes, n->nodes * sizeof(xml_node_t *));
		free(old_nodes);
		n->child_nodes[n->nodes] = child;
		n->child_nodes[n->nodes]->father = n;
		n->nodes++;
	}
}

/*
void print_xml_node(xml_node_t *n) {
	int i;
	if(n == NULL)
		return;
	if(n->name != NULL)
		printf("%s", n->name);
	for(i = 0; i < n->attributes; i++) {
		printf(" %s='%s'", n->attribute_keys[i], n->attribute_values[i]);
	}
	if(n->data != NULL)
		printf("[%s]", n->data);
	printf("\n");
	for(i = 0; i < n->nodes; i++) {
		print_xml_node(n->child_nodes[i]);
	}
}
*/

/* Parses starting tags of a XML document. */
void tag_start(void *data, const XML_Char *name, const XML_Char **attrs) {
	xml_node_t **father = (xml_node_t **) data;
	xml_node_t *child = init_xml_node();

	set_name_xml_node(child, (char *) name);
	while(*attrs) {
		add_attr_xml_node(child, (char *) attrs[0], (char *) attrs[1]);
		attrs += 2;
	}
	add_child_xml_node(*father, child);
	*father = child;
}

/* Parses ending tags of a XML document. */
void tag_end(void *data, const XML_Char *name) {
	xml_node_t **father = (xml_node_t **) data;
	*father = (*father)->father;
}

/* Parses text between tags of a XML document. */
void tag_text(void *data, const XML_Char *txt, int len) {
	xml_node_t **father = (xml_node_t **) data;
	xml_node_t *child = init_xml_node();
	char *p;

	p = (char *) &txt[len-1];
    while(p >= txt && isspace(*p)) {
		*p = '\0';
		len--;
		p--;
	}
	p = (char *) txt;
    while(p <= &txt[len-1] && isspace(*p)) {
		len--;
    	p++;
	}
	if(len > 0) {
		set_data_xml_node(child, p, len);
		add_child_xml_node(*father, child);
	}
}

/* Parses a XML document. Returns a tree structure
 * accordingly to the document. */
xml_node_t *xml_parser(char *filename) {
	XML_Parser parser;
	FILE *fp;
	int len;
	void *buff;
	xml_node_t *x;

	fp = fopen(filename, "r");
	if(!fp)
		quit("Error: Could not open file for XML parse!\n");

	parser = XML_ParserCreate(NULL);
	x = init_xml_node();
	XML_SetUserData(parser, &x);
	XML_SetElementHandler(parser, tag_start, tag_end);
	XML_SetCharacterDataHandler(parser, tag_text);

	while(!feof(fp)) {
		buff = XML_GetBuffer(parser, XML_BUFF_SIZE);
		if(!buff)
			quit("Error: Could not parse XML file!\n");
		len = fread(buff, 1, XML_BUFF_SIZE, fp);
		if(!XML_ParseBuffer(parser, len, 0))
			quit("Error: Could not parse XML file!\n");
	}
	fclose(fp);
	if(!XML_ParseBuffer(parser, 0, 1))
		quit("Error: Could not parse XML file!\n");
	XML_ParserFree(parser);

	return x;
}

/* Returns the value of a XML node's attribute. */
char *get_attribute(xml_node_t *n, char *key) {
	int i;
	if(n->attributes > 0 && n->attribute_keys != NULL)
		for(i = 0; i < n->attributes; i++)
			if(n->attribute_keys[i] != NULL && !strcmp(n->attribute_keys[i], key))
				return n->attribute_values[i];
	return NULL;
}

/* Auxilary function for get_elements_by_tag_name() below. */
xml_node_t **get_elements_by_tag_name_rec(xml_node_t *n, char *tag_name, xml_node_t **list, int *size) {
	int i;
	if(n == NULL)
		return list;
	if(n->name != NULL && !strcmp(n->name, tag_name)) {
		list = (xml_node_t **) realloc(list, (*size + 2) * sizeof(xml_node_t *));
		if(list == NULL)
			quit("Error: Could not create list of XML elements!\n");
		list[*size] = n;
		list[*size + 1] = NULL;
		(*size)++;
	}
	if(n->nodes > 0 && n->child_nodes != NULL)
		for(i = 0; i < n->nodes; i++)
			list = get_elements_by_tag_name_rec(n->child_nodes[i], tag_name, list, size);
	return list;
}

/* Returns a list of XML nodes that have the same name as the given input. 
 * The list must be free()'d after use. */
xml_node_t **get_elements_by_tag_name(xml_node_t *n, char *tag_name) {
	int size = 0;
	xml_node_t **list;
	list = (xml_node_t **) malloc(sizeof(xml_node_t *));
	if(list == NULL)
		quit("Error: Could not create list of XML elements!\n");
	list[size] = NULL;
	list = get_elements_by_tag_name_rec(n, tag_name, list, &size);
	if(size == 0) {
		free(list);
		return NULL;
	} else {
		return list;
	}
}
