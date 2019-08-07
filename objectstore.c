#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <fcntl.h>

#include<util.h>

/*variabili condivise tra il main e i vari thread worker */
static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mod=PTHREAD_COND_INITIALIZER;

//#client connessi, size totale dell'object store, #totale di oggetti
static int conn_client, tot_size, n_obj;

int main(int argc, char *argv[]){
    /*creazione della server socket */
    int serverfd;
    CHECK(serverfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");

    struct sockaddr_un ssock_addr;
    memset(&ssock_addr, '0', sizeof(ssock_addr));
    ssock_addr.sun_family=AF_UNIX;
    strncpy(ssock_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int p;
    CHECK(p, bind(serverfd, (struct sockaddr*)&ssock_addr, sizeof(ssock_addr)), "bind");
    CHECK(p, listen(serverfd, SOMAXCONN), "listen");

    /*creazione della cartella DATA */
    CHECK(p, mkdir("data", 0700), "mkdir");

    /*inizializzo le variabili globali tutte a 0*/
    conn_client=0;
    tot_size=0;
    n_obj=0;

    /*dichiaro il worker */
    pthread_t os_worker;

    int clientfd;
    do{
        CHECK(clientfd, accept(serverfd, (struct sockaddr *)NULL, NULL), "accept");
        CHECK(p, pthread_create(&os_worker, NULL, worker, NULL), "pthread create");
    }while(True);
    
    return 0;
}