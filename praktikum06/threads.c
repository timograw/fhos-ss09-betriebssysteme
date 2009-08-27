/*
Name: threads.c
	
Purpose:
	Hauptdatei Betriebssysteme Praktikumsaufgabe 6

Autoren:
	Timo Graw
	Marcel Teuber

Version:
	0.4 - Browser emulation, Testsites.txt auf Unix Zeilenumbruch geaendert
	0.3 - Speicherfehler beseitigt, Farben hinzugefuegt
 	0.2 - Threading fertig
	0.1 - start
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h> // for gettimeofday
#include <sys/time.h>

#include "threads.h"
#include "list.h"
#include "alloc.h"
#include "memcheck.h"
#include "msocket.h"
#include "console_colors.h"

int main ()
{
	queue *fifo;
	pthread_t pro;
	pthread_t *con;
	pthread_attr_t attr;
	int i;
	int consumer_count;
	int queue_size;
	thread_data_t *thread_data;
	struct timeval start, end;

	gettimeofday(&start, NULL);

	consumer_count = CONSUMER_COUNT;
	queue_size = QUEUE_SIZE;

	con = (pthread_t *)malloc(sizeof(pthread_t) * consumer_count);
	
	if (con == NULL) {
		fprintf(stderr, "main(): Malloc fehlgeschlagen");
		return 1;
	}
	
	M_SETDEBUG(DL_Information);
	
	fifo = queue_create (QUEUE_SIZE);
	if (fifo ==  NULL) {
		fprintf (stderr, "main: Queue Init failed.\n");
		exit (1);
	}
	pthread_attr_init(&attr);
	
	pthread_create (&pro, &attr, producer, fifo);
	for (i = 0; i < CONSUMER_COUNT; i++) {
		thread_data = (thread_data_t *)malloc(sizeof(thread_data_t));
		thread_data->threadid = i;
		thread_data->data = fifo;
		pthread_create (&con[i], NULL, consumer, thread_data);
	}
	
	pthread_join (pro, NULL);
	for (i = 0; i < CONSUMER_COUNT; i++)
		pthread_join (con[i], NULL);
	queue_delete (fifo);
	
	free(con);

	gettimeofday(&end, NULL);

	printf("Queuesize: %i Consumercount: %i Time: %lfs\n", QUEUE_SIZE, CONSUMER_COUNT, (end.tv_usec - start.tv_usec +
            1000000 * (end.tv_sec - start.tv_sec)) / 1000000.0);
	M_SHOW();

	return 0;
}

void *producer (void *p)
{
	queue *fifo;
	FILE *fp;
	char line[LINE_LENGTH];
	char *line_ptr;
	int ret;

	fifo = (queue *)p;
	
	fp = fopen("testSites.txt", "r");
	
	if (fp == NULL) {
		fprintf(stderr, "producer(): Konnte Datei nicht oeffnen");
		return NULL;
	}
	
	while (1) {
		ret = (int) fgets(line, LINE_LENGTH, fp);
		
		pthread_mutex_lock(fifo->mut);
		
		if (ret == 0) {
			#ifdef DEBUG
			printf("%sproducer(): Datei bearbeitet, beende Thread%s\n", BASH_RED, BASH_END);
			#endif
			
			fifo->done = 1;
			
			pthread_mutex_unlock (fifo->mut);
			
			pthread_cond_broadcast (fifo->newItemOrEnd);
			
			return NULL;
		}
		
		if (fifo->full) {
			#ifdef DEBUG
			printf("%sproducer(): Warte auf notFull signal%s\n", BASH_BROWN, BASH_END);
			#endif
			pthread_cond_wait (fifo->notFull, fifo->mut);
		}
		
		line[strlen(line)-1] = '\0';
		line_ptr = strcpy_malloc(line);

		#ifdef DEBUG
		printf("%sproducer(): Lege \"%s\" auf Queue%s\n", BASH_GREEN, line, BASH_END);
		#endif
		queue_add(fifo, line_ptr);
		
		pthread_mutex_unlock (fifo->mut);

		pthread_cond_signal (fifo->newItemOrEnd);
	}
	
	return NULL;
}

void *consumer (void *q)
{
	thread_data_t *thread_data;
	queue *fifo;
	char *url;
	int fileNr;

	thread_data = (thread_data_t *)q;
	fifo = thread_data->data;

	#ifdef DEBUG
	printf("consumer()[%i]: starte Thread\n", thread_data->threadid);
	#endif
	
	pthread_mutex_lock (fifo->mut);
	
	while (1) {
		
		if (fifo->empty) {
			/* Endbedingung */
			if (fifo->done) {
				#ifdef DEBUG
				printf("%sconsumer()[%i]: beende Thread%s\n", BASH_LIGHT_RED, thread_data->threadid, BASH_END);
				#endif
				pthread_mutex_unlock(fifo->mut);
				free(thread_data);
				return NULL;
			}
			else {
				printf ("%sconsumer()[%i]: Warte auf newItemOrEnd signal%s\n", BASH_YELLOW, thread_data->threadid, BASH_END);
				pthread_cond_wait (fifo->newItemOrEnd, fifo->mut);
			}
		}
		else {
			if (queue_remove (fifo, &url) != 0) {
				// Fehler aufgetreten, Thread beenden
				free(thread_data);
				pthread_mutex_unlock(fifo->mut);
				return NULL;
			}
			fileNr = fifo->filenr++;
			
			pthread_cond_signal (fifo->notFull);
			pthread_mutex_unlock (fifo->mut);
			
			printf ("%sconsumer()[%i]: bearbeite \"%s\"%s\n", BASH_LIGHT_BLUE, thread_data->threadid, url, BASH_END);
			http_download(url, fileNr);
			free(url);
			
			pthread_mutex_lock(fifo->mut);
		}
	}
	
}


queue *queue_create(int size)
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) {
		fprintf(stderr, "queue_create(): Malloc nicht erfolgreich\n");
		return NULL;
	}

	q->empty = 1;
	q->full = 0;
	q->done = 0;
	q->size = size;
	q->filenr = 0;
	
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->newItemOrEnd = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->newItemOrEnd, NULL);
	
	q->list = list_create();
	
	return q;
}

void queue_delete (queue *q)
{
	list_delete(q->list);
	
	pthread_mutex_destroy (q->mut);
	free (q->mut);	
	pthread_cond_destroy (q->notFull);
	free (q->notFull);
	pthread_cond_destroy (q->newItemOrEnd);
	free (q->newItemOrEnd);
	free (q);
}

int queue_add(queue *q, char *line)
{
	if (q == NULL || line == NULL || q->list == NULL) {
		fprintf(stderr, "queue_add(): Nullpointer uebergeben\n");
		return 1;
	}
	
	list_append(q->list, line);
	
	if (q->list->size == q->size)
		q->full = 1;
	q->empty = 0;
	
	return 0;
}

int queue_remove (queue *q, char **out)
{
	
	if (q == NULL || out == NULL || q->list == NULL) {
		fprintf(stderr, "queue_remove(): Nullpointer uebergeben\n");
		return 1;
	}
	
	if (q->list->size == 0) {
		fprintf(stderr, "queue_remove(): Schlange leer\n");
		return 2;
	}
	
	*out = q->list->head->data;
	list_remove(q->list, q->list->head);

	if (q->list->size == 0)
		q->empty = 1;
	q->full = 0;
	
	return 0;
}

int http_download(char *url, int filenr) {
	char filename[100];
	char address[100];
	char page[100];
	char *token, *saveptr;
	int n;
	
	if (url == NULL || filenr < 0) {
		fprintf(stderr, "http_download(): Nullpointer oder negative filenr uebergeben\n");
		return 1;
	}
	
	n = sprintf(filename, "download/file%d.html", filenr);
	
	token = strtok_r(url, " ", &saveptr);
	if (token == NULL) {
		fprintf(stderr, "http_download(): strtok_r[0] fehlgeschlagen\n");
		return 1;
	}
	strcpy(address, token);
	
	token = strtok_r(NULL, " ", &saveptr);
	if (token == NULL) {
		fprintf(stderr, "http_download(): strtok_r[1] fehlgeschlagen\n");
		return 1;
	}
	strcpy(page, token);
	
	return askServer(address, page, filename);
}

