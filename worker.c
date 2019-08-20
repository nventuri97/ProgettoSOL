#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<worker.h>
#include<sys/stat.h> 

void Worker(int client_fd){
    /*alloco in questo modo poiché le parole chiave hanno lunghezza massima di 8 più uno spazio*/
    char cl_msg[9+MAXNAME+2];
    int p;
    CHECK(p, read(client_fd, cl_msg, strlen(cl_msg)+1), "read");
    
    /*devo capire quale sia la richiesta da parte del client, in base a quella scelgo l'azione da fare*/
    char *cont;
    char *keyword=strtok_r(cl_msg, " ", &cont);
    if(strcmp(keyword,"REGISTER")==0)
        Wregister(cont, client_fd);
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
        CHECK(p, write(client_fd, answer_msg, strlen(answer_msg)*sizeof(char)+1), "write");
    }
}

void Wregister(char *cont, int client_fd){
    char *cl_name=strtok_r(cont,"\n", &cont);
    char *userpath;
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
        CHECK(p, sprintf(userpath, "%s/%s", "data", cl_name), "sprintf");
        mkdir(userpath, 0777);
        /*Invio il messaggio di riuscita connessione*/
        CHECK(p, write(client_fd, "OK \n", 3*sizeof(char)), "write");
        conn_client++;
    } else if(curr->connected==0){
        curr->connected=1;
        curr->workerfd=client_fd;

        /*Invio il messaggio di riuscita connessione, cliente già connesso precedentemente*/
        CHECK(p, write(client_fd, "OK \n", 3*sizeof(char)), "write");
        conn_client++;
    } else if(curr->connected==1){
        /*Invio messaggio di fallimento di connessione*/
        char *response;
        int err=sprintf(response, "%s", "KO, client già connesso con questo nome \n");
        CHECK(p, write(client_fd, response, strlen(response)), "write");
    }

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void Wstore(char *cont, int client_fd){
    char *filename;
    worker_t *curr=worker_l;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);

    ready=0;
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;
    int err;
    /*Creo il nome del file inserendolo direttamente nella cartella del client*/
    CHECK(err, sprintf(filename, "%s/%s/%s", "data", curr->_name, strtok_r(cont, " ", &cont)), "sprintf");
    char *end; 
    long int len=strtol(strtok_r(cont, " ", &cont), &end, 10);
    tot_size+=len;

    char *buffer=(char*) calloc(len+1, sizeof(char));
    int f_fd;
    /*Apro il file in lettura/scrittura con l'opzione che deve essere creato se non esistente*/
    CHECK(f_fd, open(filename, O_CREAT|O_RDWR, 0777), "open");
    /*Elimino dal messaggio " \n "*/
    end=strtok_r(cont, " ", &cont);
    CHECK(err, sprintf(buffer, "%s", cont[1]), "sprintf");

    /*Leggo sul canale socket la restante parte del messaggio che non ho letto con fgets poi lo scrivo nel file effettivo*/
    CHECK(err, read(client_fd, buffer, len+1-strlen(buffer)), "read");
    CHECK(err, write(f_fd, buffer, strlen(buffer)), "write");

    if(err!=-1){
        n_obj++;
        CHECK(err, write(client_fd, "OK \n", 4), "write");
    } else {
        char *response;
        CHECK(err, sprintf(response, "%s", "KO, salvataggio file non riuscito"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)), "write");
    }

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void Wretrieve(char *cont, int client_fd){
    char *filename, *path;
    worker_t *curr;
    int err;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    
    /*Ricreo il path del file*/
    filename=strtok_r(cont, " ", &cont);
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;
    
    CHECK(err, sprintf(path, "%s/%s/%s", "data", curr->_name, filename), "sprintf");
    struct stat info;

    /*Devo controllare che il file effettivamente ci sia*/
    int f_fd;
    CHECK(f_fd, open(path, O_RDONLY), "open");

    char *response;
    if(f_fd<0){
        CHECK(err, sprintf(response, "%s", "KO il file che hai cercato non esiste \n"), "sprintf");
        CHECK(err, write(client_fd, response, strlen(response)), "write");
        return;
    }
    /*Il file esiste e quindi devo andare a leggerlo*/
    CHECK(err, stat(path, &info), "stat");
    off_t len=info.st_size;

    char buffer[len+1];
    /*Leggo il file e lo salvo nel buffer*/
    CHECK(err, read(f_fd, buffer, len+1), "read");
    CHECK(err, sprintf(response, "%s %s \n %s", "OK", len, buffer), "sprintf");

    CHECK(err, write(client_fd, response, strlen(response)), "write");

    ready=1;
    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);
}

void Wdelete(char *cont, int client_fd){
    char *filename;
    worker_t *curr;

    /*Lavoro in mutua esclusione*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod,&mtx);
    
    ready=0;
    while(curr->workerfd!=client_fd)
        curr=curr->nxt;

    char *path;
    filename=strtok_r(cont, " ", cont);
    int err;
    CHECK(err, sprintf(path, "%s/%s/%s", "data", curr->_name, filename), "sprintf");
    /*Devo prendere la lunghezza per poi toglierla dalla dimensione totale dell'objectstore*/
    struct stat info;
    CHECK(err, stat(path, &info), "stat");
    off_t len=info.st_size;
    CHECK(err, unlink(path), "unlink");

    char *response;
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
    CHECK(p, write(client_fd, "OK \n", 4), "write");
    CHECKSOCK(p, close(client_fd), "close");
}

int main(){
    return 0;
}