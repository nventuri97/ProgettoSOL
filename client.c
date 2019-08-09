#define _POSIX_C_SOURCE 200112L
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
void print_report();

int main(int argc, char *argv[]){
    if(argc<3){
        fprintf(stderr, "Use, %s name action\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*name lenght, object name lenght */
    int nl;

    if((nl=strlen(argv[1]))>MAXNAME){
        fprintf(stderr, "Il nome deve essere < di %d\n", MAXNAME);
        return EXIT_FAILURE;
    }

    char *name;
    name=calloc(nl, sizeof(char));
    strcpy(name,argv[1]);

    int err, i=0;
    /*Provo a connettermi al server per 5 volte, con 5 scelta arbitraria, in caso di mancata connessione riporto l'errore */
    while(i<5 && err!=True){
        err=os_connect(name);
        i++;
    }
    int action=atoi(argv[3]);

    success=0;
    failure=0;
    tot_test=0;
    

    if(err==True){
        success++;
        tot_test++;
        if(action==1)
            store_test();
        else if(action==2)
            retrieve_test();
        else if(action==3)
            delete_test();
        else{
            /*faccio una disconnessione perché altrimenti sul server ci sarebbe un client morto in quanto ho sbagliato action*/
            int x=os_disconnect();
            if(x==False){
                failure++;
                tot_test++;
                fprintf(stderr, "Disconnessione fallita");
            }
            fprintf(stderr, "Action deve essere compreso tra 1 e 3\n");
            return EXIT_FAILURE;
        }
    } else {
        failure++;
        tot_test++;
        fprintf(stderr, "Tentativi di connessione esauriti\n");
        perror("connect");
        print_report();
        return EXIT_FAILURE;
    }

    /*action avvenuta, adesso posso disconnettermi dal server*/
    err=os_disconnect();
    if(err==False){
        fprintf(stderr, "Disconnessione fallita");
        failure++;
        tot_test++;
    } else {
        success++;
        tot_test++;
    }
    

    print_report();
    return 0;
}

void print_report(){
    fprintf(stdout, "REPORT FINALE:\n");
    fprintf(stdout, "1) Tentativi conclusi con successo: %d\n 2) Tentativi falliti: %d\n 3) Tentativi totali %d\n", success, failure, tot_test);
}