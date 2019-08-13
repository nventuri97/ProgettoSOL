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
        CHECK(err, writen(sockfd,(char *) msg, strlen(msg)*sizeof(char)), "writen");
        if(err==-1)
            return False;

        /*Messaggio di risposta*/
        int l;
        char *answer=(char*) calloc(l,sizeof(char));
        /*aspetto con il primo messaggio la lunghezza effettiva della risposta*/
        CHECK(err, readn(sockfd, (int *) &l, sizeof(int)), "readn");
        if(err==-1)
            return False;
        
        CHECK(err, readn(sockfd, (char *) answer, l*sizeof(char)+1), "readn");
        if(strncmp(answer,"OK", 2)==0)
            return True;
        else{
            fprintf(stderr, "Connessione: %s\n", answer);
            return False;
        }
    }
}

int os_store(char *name, void *block, size_t len){

}

void *os_retrieve(char *name){

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