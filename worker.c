#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>

#include"worker.h"

worker_t* w_register(char *cont, int client_fd){
    char *cl_name=strtok_r(cont," ", &cont);
    char userpath[UNIX_PATH_MAX];

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;

    int err;
    char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);

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
            if((curr->nxt)!=NULL)
                (curr->nxt)->prv=new_worker;
            new_worker->nxt=curr->nxt;
            if((curr->prv)!=NULL)
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
    
    return new_worker;
}

void w_store(char *cont, worker_t *cl_curr){
    char filepath[UNIX_PATH_MAX];
    int b_read=6;
    int err;

    char* filename=strtok_r(cont, " ", &cont);
    b_read+=strlen(filename)+1;             //devo considerare lo spazio con +1
    /*Costruisco il path del file che devo salvare*/
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", cl_curr->_name, filename), "sprintf");

    /*Prendo la lunghzza del file e la converto*/
    char *end=strtok_r(cont, " ", &cont);
    b_read+=strlen(end)+3;
    int len=strtol(end, NULL, 10);

    /*Elimino il \n, dopo mi rimangono solamente i dati*/
    end=strtok_r(cont, " ", &cont);

    char f_buffer[len+1];
    memset(f_buffer, 0, len+1);
    int f_fd;                               //file descriptor del file che devo salvare
    /*Apro il file con opzione RDWR, CREAT*/
    CHECK(f_fd, open(filepath, O_CREAT|O_RDWR, 0777), "open");

    b_read=MAXBUFSIZE-b_read;
    if(len<=b_read){
        /*I dati sono tutti nell'header*/
        CHECK(err, writen(f_fd, cont, len), "writen");
    } else {
        CHECK(err, writen(f_fd, cont, b_read), "writen");
        /*Leggo la restante parte dei dati e li scrivo sul file*/
        int rest=len-b_read;
        CHECK(err, read(cl_curr->workerfd, f_buffer, rest), "read");
        CHECK(err, writen(f_fd, f_buffer, rest), "writen");
        /*Chiudo il file descriptor*/
        CHECK(err, close(f_fd), "close");
    }

    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    ready=0;
    char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);
    if(err!=-1){
        tot_size+=len;
        n_obj++;
        CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
        CHECK(err, writen(cl_curr->workerfd, response, strlen(response)*sizeof(char)), "writen");
    } else {
        CHECK(err, sprintf(response, "%s", "KO salvataggio file non riuscito \n"), "sprintf");
        CHECK(err, writen(cl_curr->workerfd, response, strlen(response)*sizeof(char)), "writen");
    }
    printf("Store: %s", response);
    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void w_retrieve(char *cont, worker_t *cl_curr){
    char *filename, filepath[UNIX_PATH_MAX];
    int err;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    
    /*Ricreo il path del file*/
    filename=strtok_r(cont, " ", &cont);
    
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", cl_curr->_name, filename), "sprintf");

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
    /*Rilascio la mutua esclusione in quanto non vado a toccare più variabili condivise*/
    struct stat info;

    /*Devo controllare che il file effettivamente ci sia*/
    int f_fd;
    CHECK(f_fd, open(filepath, O_RDONLY), "open");

    char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);
    if(f_fd<0){
        CHECK(err, sprintf(response, "%s", "KO il file che hai cercato non esiste \n"), "sprintf");
        CHECK(err, write(cl_curr->workerfd, response, strlen(response)), "write");
        return;
    }
    /*Il file esiste e quindi devo andare a leggerlo*/
    CHECK(err, stat(filepath, &info), "stat");
    off_t len=info.st_size;

    char data[len+1];
    char buffer[MAXBUFSIZE+len+1];
    /*Leggo il file e lo salvo nel buffer*/
    CHECK(err, read(f_fd, buffer, len+1), "read");
    CHECK(err, sprintf(buffer, "%s %ld \n %s", "OK", len, data), "sprintf");

    CHECK(err, write(cl_curr->workerfd, buffer, strlen(buffer)), "write");
}

void w_delete(char *cont, worker_t *cl_curr){
    char *filename;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    
    ready=0;

    char filepath[UNIX_PATH_MAX];
    filename=strtok_r(cont, " ", &cont);
    int err;
    CHECK(err, sprintf(filepath, "%s/%s/%s", "data", cl_curr->_name, filename), "sprintf");
    printf("%s\n", filepath);

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
    /*Rilascio la mutua esclusione in quanto non vado a toccare più variabili condivise*/
    /*Devo prendere la lunghezza per poi toglierla dalla dimensione totale dell'objectstore*/
    struct stat info;
    CHECK(err, stat(filepath, &info), "stat");
    off_t len=info.st_size;
    CHECK(err, unlink(filepath), "unlink");

    char response[MAXBUFSIZE];
    memset(response, 0, MAXBUFSIZE);
    pthread_mutex_lock(&mtx);
    if(err==0){
        CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
        CHECK(err, write(cl_curr->workerfd, response, strlen(response)), "write");
        n_obj--;
        tot_size-=(int) len;
    } else {
        CHECK(err, sprintf(response, "%s", "KO, rimozione file fallita \n"), "sprintf");
        CHECK(err, write(cl_curr->workerfd, response, strlen(response)), "write");
    }
    pthread_mutex_unlock(&mtx);
}

void w_leave(worker_t *cl_curr){
    /*Gestisco la mia lista in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;

    cl_curr->connected=0;
    conn_client--;

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);

    int err;
    char response[5];
    memset(response, 0, 5);
    CHECK(err, sprintf(response, "%s", "OK \n"), "sprintf");
    CHECK(err, write(cl_curr->workerfd, response, 5), "write");
    CHECKSOCK(err, close(cl_curr->workerfd), "close");
}

void *worker(void *cl_fd){
    /*alloco in questo modo poiché le parole chiave hanno lunghezza massima di 8 più uno spazio*/
    long int client_fd=(long) cl_fd;
    char cl_msg[MAXBUFSIZE];
    int err;
    char *keyword;
    worker_t *cl_curr=NULL;
    do {
        memset(cl_msg,0, MAXBUFSIZE);
        CHECK(err, read(client_fd, cl_msg, MAXBUFSIZE), "read");
        
        /*devo capire quale sia la richiesta da parte del client, in base a quella scelgo l'azione da fare*/
        char *cont;
        keyword=strtok_r(cl_msg, " ", &cont);
        printf("%s---%s\n", keyword, cont);
        if(strcmp(keyword,"REGISTER")==0)
            cl_curr=w_register(cont, client_fd);
        else if(strcmp(keyword,"STORE")==0)
            w_store(cont, cl_curr);                    
        else if(strcmp(keyword,"RETRIEVE")==0)
            w_retrieve(cont, cl_curr);                 
        else if(strcmp(keyword,"DELETE")==0)
            w_delete(cont, cl_curr);    
        else if(strcmp(keyword, "LEAVE")==0){
            w_leave(cl_curr);
            break;
        }else{
            char answer_msg[MAXBUFSIZE];
            memset(answer_msg, 0, MAXBUFSIZE);
            CHECK(err, sprintf(answer_msg, "%s", "KO keyword errata"), "sprintf");
            CHECK(err, write(client_fd, answer_msg, strlen(answer_msg)*sizeof(char)+1), "write");
        }
    } while(True);
    pthread_exit(NULL);
}