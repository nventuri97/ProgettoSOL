#define _POSIX_C_SOURCE 200112L
#include<access.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>

/*Dichiaro come variabile globale il file descriptor in quanto dovrà essere usato in più funzioni,
non solamente nella os_connect */
static int sockfd;

int os_connect(char *name){
    struct sockaddr_un serveraddr;
    CHECK(sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");
    memset(&serveraddr, '0', sizeof(serveraddr));

    serveraddr.sun_family=AF_UNIX;
    strncpy(serveraddr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    /*uso la connect per connettermi al server */
    int err=connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    if(err==-1)
        return False;
    else{
        /*Se la connect è andata a buon fine allora */
        char *msg=(char*) calloc(9+strlen(name)+1, sizeof(char));
        strncpy(msg, "REGISTER ", 9);
        strncat(msg, name, strlen(name));

        /*Richiesta di iscrizione*/
        CHECKSOCK(err, writen(sockfd,(char *) msg, strlen(msg)*sizeof(char)), "writen");
        if(err==-1)
            return False;

        /*Messaggio di risposta*/
        char answer[MAXBUFSIZE];
        /*aspetto con il primo messaggio la lunghezza effettiva della risposta*/
        
        CHECKSOCK(err, readn(sockfd, (char *) answer, MAXBUFSIZE), "readn");
        if(strncmp(answer,"OK", 2)==0)
            return True;
        else{
            fprintf(stderr, "Connessione: %s\n", answer);
            return False;
        }
    }
}

int os_store(char *name, void *block, size_t len){
    /*Devo decidere se mettere i controlli sul nome e su block*/

    /*Messaggio dove inserirò STORE name len \n block*/
    char *msg=(char*) calloc(MAXBUFSIZE+len+1, sizeof(char));
    int err=sprintf(msg, "%s %s %ld \n %s", "STORE", name, len, (char *)block);

    size_t msglen=strlen(msg);
    /*Invio il file da salvare*/
    CHECKSOCK(err, writen(sockfd, msg, msglen), "writen");

    /*Messaggio di risposta del server*/
    char answer[MAXBUFSIZE];
    CHECKSOCK(err, readn(sockfd, answer, MAXBUFSIZE), "readn");

    if(strncmp(answer, "OK", 2)==0)
        return True;
    else{
        fprintf(stderr, "Salvataggio: %s\n", answer);
        return False;
    }
}

void *os_retrieve(char *name){
    /*Devo decidere se mettere i controlli sul nome del file*/

    /*Messaggio dove inserisco retrieve e nome del file da recuperare*/
    char msg[9+strlen(name)+1];
    strncpy(msg, "RETRIEVE ", 9);
    strcat(msg,name);

    int err;
    /*Invio il messaggio al server*/
    CHECKSOCK(err, writen(sockfd, msg, strlen(msg)), "writen");

    /*Messaggio di risposta dal server*/
    char answer[MAXBUFSIZE];
    CHECKSOCK(err, readn(sockfd, answer, MAXBUFSIZE), "readn");

    char *cont=NULL;
    char *ansmsg=strtok_r(answer, " ", &cont);
    void *file=NULL;

    if(strcmp(ansmsg,"DATA")==0){
        /*Verifico che cont contenga i dati necessari*/
        if(cont!=NULL){
            char *end, *support;
            /*Prendo la lunghezza del essaggio*/
            support=strtok_r(cont, "\n", &cont);
            long int len=strtol(support, &end, 10);

            file=(char*) calloc(len, sizeof(char));
            CHECKSOCK(err, readn(sockfd, file, len), "readn");
        }
        return file;
    } else if(strcmp(ansmsg, "KO")==0)
        fprintf(stderr, "Lettura: KO %s\n", cont);
    return file;
}

int os_delete(char *name){

}

int os_disconnect(){
    int err;
    CHECK(err, writen(sockfd, "LEAVE", 6*sizeof(char)), "writen");
    if(err==-1)
        return False;

    char answer[3];
    CHECK(err, readn(sockfd, (char*) answer, 3*sizeof(char)), "readn");
    if(err==-1)
        return False;
    fprintf(stdout, "Disconnessione: %s\n", answer);
    CHECK(err, close(sockfd), "close");
    if(err==-1)
        return False;
    return True;
}

int main(){
    return 0;
}