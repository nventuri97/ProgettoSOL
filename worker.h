#ifndef WORKER_H
#define WORKER_H

#define _POSIX_C_SOURCE 200809L
#include"util.h"
#include<signal.h>
#include<dirent.h>

/*variabili condivise tra il main e i vari thread worker */
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

//#client connessi, size totale dell'object store, #totale di oggetti
static volatile sig_atomic_t conn_client, tot_size, n_obj;
static volatile sig_atomic_t serveronline=1;

/*struct per definire la lista di worker collegati ad un client*/
typedef struct worker{
    struct worker *nxt;
    struct worker *prv;
    int workerfd;
    int connected;
    char _name[MAXNAME+1];
}worker_t;

worker_t *worker_l;

void *worker(void *cl_fd);

#endif