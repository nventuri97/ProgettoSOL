#define _POSIX_C_SOURCE 200112L
#include<unistd.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>

#include<util.h>
#include<access.h>

#define MAXNAME 100

static int success, failure, tot_test;

void store_test();
void retrieve_test();
void delete_test();

int main(int argc, char *argv[]){
    if(argc<3){
        fprintf(stderr, "Use, %s name action\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*name lenght, object name lenght */
    int nl, onl;

    if((nl=strlen(argv[1]))>MAXNAME){
        fprintf(stderr, "Il nome deve essere < di %d\n", MAXNAME);
        return EXIT_FAILURE;
    }

    char *name;
    name=calloc(nl, sizeof(char));
    strcpy(name,argv[1]);

    int err;
    /*Provo a connettermi al server per 5 volte, con 5 scelta arbitraria */
    for(int i=0;i<5;i++)
        err=os_connect(name);
    int action=atoi(argv[3]);

    success=0;
    failure=0;
    tot_test=0;
    

    if(err==True){
        if(action==1)
            store_test();
        else if(action==2)
            retrieve_test();
        else if(action==3)
            delete_test();
        else{
            fprintf(stderr, "Action deve essere compreso tra 1 e 3\n");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Tentativi di connessione esauriti\n");
        perror("connect");
        return EXIT_FAILURE;
    }

    return 0;
}