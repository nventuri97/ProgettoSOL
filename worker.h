#ifndef WORKER_H
#define WORKER_H

#define _POSIX_C_SOURCE 200112L
#include<util.h>

/*variabili condivise tra il main e i vari thread worker */
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mod=PTHREAD_COND_INITIALIZER;

//#client connessi, size totale dell'object store, #totale di oggetti
static int conn_client, tot_size, n_obj, ready=1;

/*struct per definire la lista di worker collegati ad un client*/
typedef struct worker{
    struct worker *nxt;
    struct worker *prv;
    int workerfd;
    int connected;
    char _name[MAXNAME+1];
    pthread_t pid;
}worker_t;

worker_t *worker_l;

void *Worker(int client_fd);

#endif