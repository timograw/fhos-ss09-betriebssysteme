/*
Name: list.c
	
Purpose:
	Doppelverlinkte Liste fuer beliebigen Inhalt (void Zeiger)

Autoren:
	Timo Graw
	Marcel Teuber

Version:
	0.1 - start
*/

#include <stdio.h>
#include <stdlib.h>

#include "list.h"

list_t *list_create() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	
	return list;
}

int list_append(list_t *list, void *data) {
	if (list == NULL || data == NULL) {
		fprintf(stderr, "list_append_node(): Nullpointer uebergeben");
		return 1;
	}
	
	node_t *node = (node_t *)malloc(sizeof(node_t));
	
	node->data = data;
	
	return list_append_node(list, node);
}

int list_append_node(list_t *list, node_t *lnode) {
	if (list == NULL || lnode == NULL) {
		fprintf(stderr, "list_append_node(): Nullpointer uebergeben");
		return 1;
	}
	
	if (list->head == NULL) {
		list->head = lnode;
		lnode->prev = NULL;
	}
	else {
		list->tail->next = lnode;
		lnode->prev = list->tail;
	}

	lnode->next = NULL;
	
	list->tail = lnode;
	list->size++;
	
	return 0;
}

void list_insert(list_t *list, node_t *lnode, node_t *after) {
	lnode->next = after->next;
	lnode->prev = after;
	
	if(after->next != NULL)
		after->next->prev = lnode;
	else
		list->tail = lnode;

	after->next = lnode;
 	list->size++;
}

void list_remove(list_t *list, node_t *lnode) {
	if(lnode->prev == NULL)
		list->head = lnode->next;
	else
		lnode->prev->next = lnode->next;

	if(lnode->next == NULL)
		list->tail = lnode->prev;
	else
		lnode->next->prev = lnode->prev;
	
	free(lnode);
	list->size--;
}

int list_delete(list_t *list) {
	if (list == NULL)
		return 1;
	
	while(list->size > 0)
		list_remove(list, list->head);
	
	free(list);
	
	return 0;
}

