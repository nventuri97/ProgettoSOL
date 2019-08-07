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

#define MAXNAME 100
#define MAXOBJNAME 100

static int success, failure, tot_test;

void store_test(char *objname);
void retrive_test(char *objname);
void delte_test(char *objname);

int main(int argc, char *argv[]){
    if(argc<4){
        fprintf(stderr, "Use, %s name obj action\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    /*name lenght, object name lenght */
    int nl, onl;

    if((nl=strlen(argv[1]))>MAXNAME){
        fprintf(stderr, "Il nome deve essere < di %d\n", MAXNAME);
        return EXIT_FAILURE;
    }
    if((onl=strlen(argv[2]))>MAXOBJNAME){
        fprintf(stderr, "Il nome dell'oggetto deve essere < di %d\n", MAXOBJNAME);
        return EXIT_FAILURE;
    }

    char *name, *objname;
    name=calloc(nl, sizeof(char));
    objname=calloc(onl, sizeof(char));
    strcpy(name,argv[1]);
    strcpy(objname, argv[2]);

    int err=os_connect(name);
    int action=atoi(argv[3]);

    success=0;
    failure=0;
    tot_test=0;
    

    if(err==True){
        if(action==1)
            store_test(objname);
        else if(action==2)
            retrieve_test(objname);
        else if(action==3)
            delete_test(objname);
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