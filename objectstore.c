#define _POSIX_C_SOURCE 200112L
#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<poll.h>

#include<util.h>
#include<worker.h>


int main(int argc, char *argv[]){
    /*creazione della server socket */
    int serverfd;
    CHECKSOCK(serverfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");

    struct sockaddr_un ssock_addr;
    memset(&ssock_addr, '0', sizeof(ssock_addr));
    ssock_addr.sun_family=AF_UNIX;
    strncpy(ssock_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int p;
    CHECKSOCK(p, bind(serverfd, (struct sockaddr*)&ssock_addr, sizeof(ssock_addr)), "bind");
    CHECKSOCK(p, listen(serverfd, SOMAXCONN), "listen");

    /*creazione della cartella DATA */
    CHECK(p, mkdir("data", 0777), "mkdir");             //posso mettere anche 0700

    /*devo lavorare in mutua esclusione sulle variabili condivise*/
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&mod, &mtx);
    /*inizializzo le variabili condivise tutte a 0*/
    /*rendo ready occupata, in modo tale che se qualcuno volesse modificare si blocca sulla wait*/
    ready=0;
    conn_client=0;
    tot_size=0;
    n_obj=0;
    /*libero ready così chi deve lavorare può lavorare*/
    ready=1;

    pthread_cond_signal(&mod);
    pthread_mutex_unlock(&mtx);

    /*dichiaro il worker */
    pthread_t os_worker;

    int clientfd;
    while(True){
        CHECKSOCK(clientfd, accept(serverfd, (struct sockaddr *)NULL, NULL), "accept");
        CHECK(p, pthread_create(&os_worker, NULL, worker_(clientfd), NULL), "pthread create");   
    }
    
    return 0;
}