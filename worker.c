#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>

#include"worker.h"

void w_register(char *cont, int client_fd){
    char *cl_name=strtok_r(cont," ", &cont);
    char userpath[UNIX_PATH_MAX];

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;

    int err;
    char response[MAXBUFSIZE];

    worker_t *new_worker=(worker_t*) malloc(sizeof(worker_t));
    new_worker->connected=1;
    new_worker->workerfd=client_fd;
    CHECK(err, sprintf(new_worker->_name, "%s", cl_name), "sprintf");
    /*Nessun client ancora registrato*/
    if(worker_l==NULL){
        worker_l=new_worker;
        worker_l->nxt=NULL;
        worker_l->prv=NULL;
        /*Creo la cartella del client in cui andrò a inserire i file*/
        CHECK(err, sprintf(userpath, "%s/%s", "data", cl_name), "sprintf");
        mkdir(userpath, 0777);
        /*Invio il messaggio di riuscita connessione*/
        CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)*sizeof(char)), "write");

        conn_client++;
    }else{
        worker_t *curr=worker_l;
        /*Cerco se l'utente si è già connesso in precedenza*/
        while(strcmp(curr->_name, cl_name)!=0 && curr->nxt!=NULL)
            curr=curr->nxt;
        /*Utente non ancora connesso*/
        if(curr->nxt==NULL && strcmp(curr->_name, cl_name)!=0){
            new_worker->nxt=worker_l;
            worker_l->prv=new_worker;
            worker_l=new_worker;
            /*Creo la cartella del client in cui andrò a inserire i file*/
            CHECK(err, sprintf(userpath, "%s/%s", "data", cl_name), "sprintf");
            mkdir(userpath, 0777);
            /*Invio il messaggio di riuscita connessione*/
            CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
            CHECK(err, write(client_fd, response, strlen(response)*sizeof(char)), "write");
            conn_client++;
        } else if(curr->connected==0){
            /*Client già connesso in precedenza ma ora offline*/
            (curr->nxt)->prv=new_worker;
            new_worker->nxt=curr->nxt;
            (curr->prv)->nxt=new_worker;
            new_worker->prv=curr->prv;
            /*Invio il messaggio di riuscita connessione*/
            CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
            CHECK(err, write(client_fd, response, strlen(response)*sizeof(char)), "write");
            free(curr);
            conn_client++;
        } else if(curr->connected==1){
            /*Client online*/
            free(new_worker);
            CHECK(err, sprintf(response, "%s", "KO client già online \n"), "sprintf");
        }
    }
    
    printf("Register: %s", response);
    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void w_store(char *cont, int client_fd){
    char filepath[UNIX_PATH_MAX];

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);

    ready=0;
    worker_t *curr=worker_l;
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;
    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);

    int err;
    /*Creo il nome del file inserendolo direttamente nella cartella del client*/
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", curr->_name, strtok_r(cont, " ", &cont)), "sprintf");
    char *end; 
    long int len=strtol(strtok_r(cont, " ", &cont), &end, 10);

    char *buffer=(char*) calloc(len+1, sizeof(char));
    int f_fd;
    /*Apro il file in lettura/scrittura con l'opzione che deve essere creato se non esistente*/
    CHECK(f_fd, open(filepath, O_CREAT|O_RDWR, 0777), "open");
    /*Elimino dal messaggio " \n "*/
    end=strtok_r(cont, " ", &cont);
    CHECK(err, sprintf(buffer, "%s", cont), "sprintf");

    /*Leggo sul canale socket la restante parte del messaggio che non ho letto con fgets poi lo scrivo nel file effettivo*/
    CHECK(err, read(client_fd, buffer, len+1-strlen(buffer)), "read");
    CHECK(err, write(f_fd, buffer, strlen(buffer)), "write");

    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    ready=0;
    char response[MAXBUFSIZE];
    if(err!=-1){
        tot_size+=len;/* condition */
        n_obj++;
        CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)*sizeof(char)), "write");
    } else {
        CHECK(err, sprintf(response, "%s", "KO, salvataggio file non riuscito"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)*sizeof(char)), "write");
    }
    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
    printf("Store: %s", response);
}

void w_retrieve(char *cont, int client_fd){
    char *filename, filepath[UNIX_PATH_MAX];
    worker_t *curr=worker_l;
    int err;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    
    /*Ricreo il path del file*/
    filename=strtok_r(cont, " ", &cont);
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;
    
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", curr->_name, filename), "sprintf");
    struct stat info;

    /*Devo controllare che il file effettivamente ci sia*/
    int f_fd;
    CHECK(f_fd, open(filepath, O_RDONLY), "open");

    char response[MAXBUFSIZE];
    if(f_fd<0){
        CHECK(err, sprintf(response, "%s", "KO il file che hai cercato non esiste \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)), "write");
        return;
    }
    /*Il file esiste e quindi devo andare a leggerlo*/
    CHECK(err, stat(filepath, &info), "stat");
    off_t len=info.st_size;

    char buffer[len+1];
    /*Leggo il file e lo salvo nel buffer*/
    CHECK(err, read(f_fd, buffer, len+1), "read");
    CHECK(err, sprintf(response, "%s %ld \n %s", "OK", len, buffer), "sprintf");

    CHECK(err, write(client_fd, response, strlen(response)), "write");

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void w_delete(char *cont, int client_fd){
    char *filename;
    worker_t *curr=worker_l;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    
    ready=0;
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;

    char filepath[UNIX_PATH_MAX];
    filename=strtok_r(cont, " ", &cont);
    int err;
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", curr->_name, filename), "sprintf");
    /*Devo prendere la lunghezza per poi toglierla dalla dimensione totale dell'objectstore*/
    struct stat info;
    CHECK(err, stat(filepath, &info), "stat");
    off_t len=info.st_size;
    CHECK(err, unlink(filepath), "unlink");

    char response[MAXBUFSIZE];
    if(err==0){
        CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)), "write");
        n_obj--;
        tot_size-=(int) len;
    } else {
        CHECK(err, sprintf(response, "%s", "KO, rimozione file fallita \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)), "write");
    }

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void w_leave(int client_fd){
    /*Gestisco la mia lista in mutua esclusione*/
    worker_t *curr=worker_l;
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;

    while(curr->workerfd!=client_fd)
        curr=curr->nxt;
    curr->connected=0;
    conn_client--;

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);

    int p;
    CHECK(p, write(client_fd, "OK \n", 4), "write");
    CHECKSOCK(p, close(client_fd), "close");
}

void *worker(void *cl_fd){
    /*alloco in questo modo poiché le parole chiave hanno lunghezza massima di 8 più uno spazio*/
    long int client_fd=(long) cl_fd;
    char cl_msg[MAXBUFSIZE];
    int p;
    CHECK(p, read(client_fd, cl_msg, MAXBUFSIZE), "read");
    
    /*devo capire quale sia la richiesta da parte del client, in base a quella scelgo l'azione da fare*/
    char *cont;
    char *keyword=strtok_r(cl_msg, " ", &cont);
    if(strcmp(keyword,"REGISTER")==0)
        w_register(cont, client_fd);
    else if(strcmp(keyword,"STORE")==0)
        w_store(cont, client_fd);                    
    else if(strcmp(keyword,"RETRIEVE")==0)
        w_retrieve(cont, client_fd);                 
    else if(strcmp(keyword,"DELETE")==0)
        w_delete(cont, client_fd);                 
    else if(strcmp(keyword,"LEAVE")==0)
        w_leave(client_fd);
    else{
        char answer_msg[MAXBUFSIZE];
        CHECK(p, sprintf(answer_msg, "%s", "KO keyword errata"), "sprintf");
        CHECK(p, write(client_fd, answer_msg, strlen(answer_msg)*sizeof(char)+1), "write");
    }
    pthread_exit(NULL);
}