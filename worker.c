#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <worker.h>

void *Worker(int client_fd){
    /*alloco in questo modo poiché le parole chiave hanno lunghezza massima di 8 più uno spazio*/
    char cl_msg[9+MAXNAME+1];
    int p;
    CHECK(p, readn(client_fd, cl_msg, strlen(cl_msg)*sizeof(char)+1), "readn");
    
    /*devo capire quale sia la richiesta da parte del client, in base a quella scelgo l'azione da fare*/
    char *cont;
    char *keyword=strtok_r(keyword, " ", &cont);
    if(strcmp(keyword,"REGISTER")==0)
        registerWregister(cont, client_fd);
    else if(strcmp(keyword,"STORE")==0)
        Wstore(cont, client_fd);                    //ancora da definire
    else if(strcmp(keyword,"RETRIEVE")==0)
        Wretrieve(cont, client_fd);                 //ancora da definire
    else if(strcmp(keyword,"DELETE")==0)
        Wdelete(cont, client_fd);                   //ancora da definire
    else if(strcmp(keyword,"LEAVE")==0)
        Wleave(client_fd);                        //ancora da definire
    else{
        char *answer_msg;
        strcpy(answer_msg, "KO keyword errata");
        CHECK(p, writen(client_fd, answer_msg, strlen(answer_msg)*sizeof(char)+1), "writen");
    }
}

void Wregister(char *cont, int client_fd){
    char *cl_name=strtok_r(cont,"\n", &cont);
    /*Apro la cartella data, non importa la mutua esclusione in quanto possono lavorarci più worker contemporaneamente*/
    DIR *data=opendir("./data");
    struct dirent* file;
    worker_t *curr=worker_l;

    int p;
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    ready=0;

    while(curr->_name!=cl_name && curr->nxt!=NULL)
        curr=curr->nxt;
    /*Nuovo cliente*/
    if(curr==NULL){
        /*devo aggiungere un thread worker alla lista*/
        worker_t *new_worker=(worker_t *) malloc(sizeof(worker_t));
        if(worker_l!=NULL)
            new_worker->prv=worker_l;
        new_worker->nxt=NULL;
        new_worker->connected=1;
        new_worker->workerfd=client_fd;
        strcpy(new_worker->_name,cl_name);
        worker_l=new_worker;

        /*Creo la cartella del client in cui andrò a inserire i file*/
        mkdir(cl_name, 0777);
        /*Invio il messaggio di riuscita connessione*/
        CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
        conn_client++;
    } else if(curr->connected==0){
        curr->connected=1;
        curr->workerfd=client_fd;

        /*Invio il messaggio di riuscita connessione, cliente già connesso precedentemente*/
        CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
        conn_client++;
    } else if(curr->connected==1){
        /*Invio messaggio di fallimento di connessione*/
        char *response;
        int err=sprintf(response, "%s", "KO, client già connesso con questo nome\n");
        CHECK(p, writen(client_fd, response, strlen(response)), "writen");
    }

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void Wstore(char *cont, int client_fd){

}

void Wretrieve(char *cont, int client_fd){

}

void Wdelete(char *cont, int client_fd){

}

void Wleave(int client_fd){
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
    CHECK(p, writen(client_fd, "OK\n", 3*sizeof(char)), "writen");
    CHECKSOCK(p, close(client_fd), "close");
}

int main(){
    return 0;
}