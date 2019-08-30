#define _POSIX_C_SOURCE 200809L
#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<poll.h>

#include"util.h"
#include"worker.h"
#include"signal_t.h"

int main(int argc, char *argv[]){
    /*creazione della server socket */
    unlink(SOCKNAME);
    int serverfd, err;
    CHECKSOCK(serverfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");

    struct sockaddr_un ssock_addr;
    memset(&ssock_addr, '0', sizeof(ssock_addr));
    ssock_addr.sun_family=AF_UNIX;
    strncpy(ssock_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    
    CHECKSOCK(err, bind(serverfd, (struct sockaddr*)&ssock_addr, sizeof(ssock_addr)), "bind");
    CHECKSOCK(err, listen(serverfd, SOMAXCONN), "listen");

    /*creazione della cartella DATA */
    CHECK(err, mkdir("data", 0777), "mkdir");

    /*devo lavorare in mutua esclusione sulle variabili condivise*/
    pthread_mutex_lock(&mtx);
    /*inizializzo le variabili condivise*/

    worker_l=NULL;
    conn_client=0;
    tot_size=0;
    n_obj=0;

    pthread_mutex_unlock(&mtx);
    
    /*Creo il thread che gestisce i sengali*/
    create_signal_t();

    /*dichiaro il worker*/
    pthread_t os_worker;
    struct pollfd fds;
    fds.fd=serverfd;
    fds.events=POLLIN;

    long int clientfd;
    while(serveronline){
        if(poll(&fds, 1, 5)>=1){
            CHECKSOCK(clientfd, accept(serverfd, (struct sockaddr *)NULL, NULL), "accept");
            CHECK(err, pthread_create(&os_worker, NULL, &worker, (void *) clientfd),"pthread_create");
            CHECK(err, pthread_detach(os_worker), "pthread_detach");
        }
    }
    CHECKSOCK(err, close(serverfd), "close");
    return 0;
}