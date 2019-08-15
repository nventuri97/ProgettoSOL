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
    
    /*devo capire quale sia la richiesta da parte del client, in base a quella scelgo l'azione da fare*/
    char *cont;
    char *keyword=strtok_r(keyword, " ", &cont);
    if(strcmp(keyword,"REGISTER")==0)
        os_register(cont, client_fd);
    else if(strcmp(keyword,"STORE")==0)
        os_store(cont, client_fd);                    //ancora da definire
    else if(strcmp(keyword,"RETRIEVE")==0)
        os_retrieve(cont, client_fd);                 //ancora da definire
    else if(strcmp(keyword,"DELETE")==0)
        os_delete(cont, client_fd);                   //ancora da definire
    else if(strcmp(keyword,"LEAVE")==0)
        os_leave(client_fd);                        //ancora da definire
    else{
        char *answer_msg;
        strcpy(answer_msg, "KO keyword errata");
        CHECK(p, writen(client_fd, answer_msg, strlen(answer_msg)*sizeof(char)+1), "writen");
    }
}

void os_register(char *cont, int client_fd){
    char *cl_name=strtok_r(cont,"\n", &cont);
    /*Apro la cartella data, non importa la mutua esclusione in quanto possono lavorarci più worker contemporaneamente*/
    DIR *data=opendir("./data");
    struct dirent* file;

    int p;
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;
    /*
    * Devo ancora gestire il caso in cui l'utente tenti di connettersi con il nome di un utente già connesso,
    * devo controllare che ci sia già la cartella e poi nel caso il bit connected relativo al nome nella hash table
    * ancora da implementare
    */
    while((errno=0, file=readdir(data))!=NULL){
        if(strcmp(file->d_name, cl_name)==0){
            /*Invio il messaggio di riuscita connessione, cliente già connesso precedentemente*/
            CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
            break;
        }
    }
    /*Nuovo cliente*/
    if(p!=0){
        /*Creo la cartella del client in cui andrò a inserire i file*/
            mkdir(cl_name, 0777);
        /*Invio il messaggio di riuscita connessione*/
            CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
    }
    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void os_disconnect(int client_fd){
    int p;
    CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
    CHECK(p, close(client_fd), "close");
}

int main(){
    return 0;
}