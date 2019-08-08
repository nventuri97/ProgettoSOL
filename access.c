#define _POSIX_C_SOURCE 200112L
#include<access.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<string.h>


int os_connect(char *name){
    int sockfd;
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
        /*Devo inviare un messaggio (protocollo di comunicazione) al server per far creare la cartella del client name*/
        return True;
    }
}

int os_store(char *name, void *block, size_t len){

}

void *os_retrieve(char *name){

}

int os_delete(char *name){

}

int os_disconnect(){

}