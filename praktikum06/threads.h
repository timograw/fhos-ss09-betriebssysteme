#ifndef THREADS_H
#define THREADS_H

#define LINE_LENGTH 128
#define QUEUE_SIZE 5
#define CONSUMER_COUNT 7

#include "list.h"

typedef struct {
	list_t *list;
	int size;
	
	int full;
	int empty;
	int done;
	
	int filenr;
	
	pthread_mutex_t *mut;
	pthread_cond_t *notFull;
	pthread_cond_t *newItemOrEnd;
} queue;

typedef struct {
	int threadid;
	queue *data;
} thread_data_t;


// Funktionen
void *producer (void *args);
void *consumer (void *args);

queue *queue_create(int size);
void queue_delete (queue *q);
int queue_add(queue *q, char *line);
int queue_remove (queue *q, char **out);
int http_download(char *url, int filenr);

#endif

