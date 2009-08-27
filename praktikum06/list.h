/* list.h */
#ifndef LIST_H
#define LIST_H

typedef struct node_s {
	struct node_s		*prev;
	struct node_s		*next;
	void 				*data;
} node_t;

typedef struct list_s {
	node_t				*head;
	node_t				*tail;
	int					size;
} list_t;

list_t 	*list_create();
int		list_append(list_t *list, void *data);
int 	list_append_node(list_t *list, node_t *lnode);
void 	list_insert(list_t *list, node_t *lnode, node_t *after);
void 	list_remove(list_t *list, node_t *lnode);
int		list_delete(list_t *list);

#endif

