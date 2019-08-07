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

#include <util.h>
#include <access.h>

static int success, failure, tot_test;

int main(int argc, char *argv[]){
    if(argc<4){
        fprintf(stderr, "Use, %s name obj action\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    success=0;
    failure=0;
    tot_test=0;
    int err=os_connect(argv[1]);
    int action=atoi(argv[3]);

    if(err==True){
        if(action==1)
            store_test(argv[2]);
        else if(action==2)
            retrieve_test(argv[2]);
        else if(action==3)
            delete_test(argv[2]);
        else{
            fprintf(stderr, "Action deve essere compreso tra 1 e 3\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror(err);
        return EXIT_FAILURE;
    }

    return 0;
}