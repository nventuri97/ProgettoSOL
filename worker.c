#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <worker.h>

void *Worker(int client_fd){
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    
    ready=0;
    /*devo aggiungere un thread worker alla lista*/
    worker_t *new_worker=(worker_t *) malloc(sizeof(worker_t));
    if(worker_l!=NULL)
        new_worker->prv=worker_l;
    new_worker->nxt=NULL;
    new_worker->connected=1;
    new_worker->pid=getpid();
    new_worker->workerfd=client_fd;
    worker_l=new_worker;

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
    
    /*alloco in questo modo poiché le parole chiave hanno lunghezza massima di 8 più uno spazio*/
    char cl_msg[9+MAXNAME+1];
    int p;
    CHECK(p, readn(client_fd, cl_msg, strlen(cl_msg)*sizeof(char)+1), "readn");
    
}

int main(){
    return 0;
}