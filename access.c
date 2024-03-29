#define _POSIX_C_SOURCE 200809L
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>

#include"access.h"

/*Dichiaro come variabile globale il file descriptor in quanto dovrà essere usato in più funzioni,
non solamente nella os_connect */
static int sockfd=-1;

int os_connect(char *name){
    if(name==NULL){
        fprintf(stderr, "Nome client non valido\n");
        return False;
    }

    struct sockaddr_un serveraddr;
    CHECKSOCK(sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");
    memset(&serveraddr, '0', sizeof(serveraddr));

    serveraddr.sun_family=AF_UNIX;
    strncpy(serveraddr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    /*uso la connect per connettermi al server */
    while(connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr))==-1)
        if(errno==ENOENT)
            sleep(1);
        else
            return False;
    
    /*Se la connect è andata a buon fine allora */
    int err;
    char msg[MAXBUFSIZE];
    memset(msg, 0, MAXBUFSIZE);
    CHECK(err, sprintf(msg, "%s %s \n","REGISTER", name), "sprintf");

    /*Richiesta di iscrizione*/
    CHECK(err, writen(sockfd, msg, strlen(msg)), "writen");
    if(err==-1)
        return False;

    /*Messaggio di risposta*/
    char answer[MAXBUFSIZE+1];
    memset(answer, 0, MAXBUFSIZE+1);
    /*aspetto con il primo messaggio la lunghezza effettiva della risposta*/
    
    CHECK(err, read_to_new(sockfd, answer, MAXBUFSIZE), "read");
    printf("Connect: %s", answer);
    if(strncmp(answer,"OK", 2)==0)
        return True;
    else{
        fprintf(stderr, "Connessione: %s\n", answer);
        return False;
    }
}

int os_store(char *name, void *block, size_t len){
    if(sockfd==-1){
        fprintf(stderr, "Socket non ancora aperta");
        return False;
    }

    if(name==NULL){
        fprintf(stderr, "Nome oggetto non valido\n");
        return False;
    }

    if(block==NULL){
        fprintf(stderr, "Oggetto non valido\n");
        return False;
    }

    if(len<0){
        fprintf(stderr, "Dimensione oggetto non valida\n");
        return False;
    }

    /*Messaggio dove inserirò STORE name len \n block*/
    char msg[MAXBUFSIZE+len+1];
    memset(msg, 0, MAXBUFSIZE+len+1);
    int err;
    CHECK(err, sprintf(msg, "%s %s %ld \n %s", "STORE", name, len, (char *)block), "sprintf");

    size_t msglen=strlen(msg);
    /*Invio il file da salvare*/
    CHECK(err, writen(sockfd, msg, msglen), "writen");
    /*Messaggio di risposta del server*/
    char answer[MAXBUFSIZE+1];
    memset(answer, 0, MAXBUFSIZE+1);
    CHECK(err, read_to_new(sockfd, answer, MAXBUFSIZE), "read");

    if(strncmp(answer, "OK", 2)==0)
        return True;
    else{
        fprintf(stderr, "Salvataggio: %s\n", answer);
        return False;
    }
}

void *os_retrieve(char *name){
    if(sockfd==-1){
        fprintf(stderr, "Socket non ancora aperta");
        return False;
    }
    
    if(name==NULL){
        fprintf(stderr, "Nome oggetto non valido\n");
        return False;
    }

    /*Messaggio dove inserisco retrieve e nome del file da recuperare*/
    int err;
    char msg[MAXBUFSIZE];
    memset(msg, 0, MAXBUFSIZE);
    CHECK(err, sprintf(msg, "%s %s \n","RETRIEVE", name), "sprintf");
    /*Invio il messaggio al server*/
    CHECK(err, writen(sockfd, msg, strlen(msg)), "writen");
 
    /*Messaggio di risposta dal server*/
    char answer[MAXBUFSIZE+1];
    memset(answer, 0, MAXBUFSIZE+1);
    CHECK(err, read_to_new(sockfd, answer, MAXBUFSIZE), "read");

    char *cont=NULL;
    char *ansmsg=strtok_r(answer, " ", &cont);
    void *file=NULL;
    
    if(strcmp(ansmsg,"DATA")==0){
        int b_read=5;
        /*Prendo la lunghezza del file che mi è stato restituito*/
        char *end=strtok_r(cont, " ", &cont);
        b_read+=strlen(end)+3;
        int len=strtol(end, NULL, 10);
        file=(char*) calloc(len+1, sizeof(char));
        end=strtok_r(cont, " ", &cont);
        b_read=MAXBUFSIZE-b_read;
        if(len<=b_read){
            strncat(file, cont, b_read);
            //printf("file: %s\n", (char*)file);
        } else {
            strncat(file, cont, b_read);            
            //printf("2.cont: %s\n", cont);
            int rest=len-b_read;
            char tmp[rest+1];
            CHECK(err, readn(sockfd, tmp, rest*sizeof(char)), "readn");
            strncat(file, tmp, rest);
        }
    }else
        fprintf(stderr, "Lettura: %s %s", ansmsg, cont);
    return file;
}

int os_delete(char *name){
    if(sockfd==-1){
        fprintf(stderr, "Socket non ancora aperta");
        return False;
    }
    
    if(name==NULL){
        fprintf(stderr, "Nome oggetto non valido\n");
        return False;
    }

    /*Messaggio dove inserisco delete e nome del file da recuperare*/
    char msg[MAXBUFSIZE];
    memset(msg, 0, MAXBUFSIZE);
    int err;
    CHECK(err, sprintf(msg, "%s %s \n", "DELETE", name), "sprintf");
    CHECK(err, writen(sockfd, msg, strlen(msg)), "writen");

    /*Messaggio di risposta dal server*/
    char answer[MAXBUFSIZE+1];
    memset(answer, 0, MAXBUFSIZE+1);
    CHECK(err, read_to_new(sockfd, answer, MAXBUFSIZE), "read");
    
    if(strncmp(answer, "OK", 2)==0)
        return True;
    else{
        fprintf(stderr, "Rimozione: %s", answer);
        return False;
    }
}

int os_disconnect(){
    if(sockfd==-1){
        fprintf(stderr, "Socket non ancora aperta");
        return False;
    }

    int err;
    CHECK(err, writen(sockfd, "LEAVE \n", 7), "write");
    if(err==-1)
        return False;

    char answer[5];
    memset(answer, 0, 5);
    CHECK(err, read_to_new(sockfd, answer, 5), "read");
    if(err==-1)
        return False;
    fprintf(stdout, "Disconnessione: %s", answer);
    CHECKSOCK(err, close(sockfd), "close");
    if(err==-1)
        return False;
    return True;
}