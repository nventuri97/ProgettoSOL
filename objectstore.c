#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include<util.h>

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

    /*dichiaro i thread dispatcher e worker */
    pthread_t os_dispatcher, os_worker;
    CHECK(p, pthread_create(os_dispatcher,NULL, dispatcher, NULL), "thread create");
    /*DEVO ANCORA DEFINIRE LA DISPATCHER NELLA LIBRERIA!!!!!!!!!!!!! */
    return 0;
}