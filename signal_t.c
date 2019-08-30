#define _POSIX_C_SOURCE 200809L
#include"signal_t.h"

void s_print_report(){
    fprintf(stdout, "SIGUSR1 catturato\n");
    fprintf(stdout, "Client connessi: %d\n", conn_client);
    fprintf(stdout, "Oggetti memorizzati nell'object store: %d\n", n_obj);

    if(tot_size>1024)
        fprintf(stdout, "Dimensione dell'objectstore: %d Mb\n", tot_size/1024);
    else
        fprintf(stdout, "Dimensione dell'objectstore: %d Kb\n", tot_size);
    
}

void *signaller(){
    /*Resetto la maschera dei segnali per poi reimpostarla solamente su SIGINT, SIGTERM, SIGUSR1*/
    sigset_t mask;
    int err, sig;
    CHECK(err, sigemptyset(&mask), "signemptyset");
    CHECK(err, sigaddset(&mask, SIGINT), "sigaddset");
    CHECK(err, sigaddset(&mask, SIGTERM), "sigaddset");
    CHECK(err, sigaddset(&mask, SIGUSR1), "sigaddset");

    while(serveronline){
        /*Blocco i segnali che ho impostato in mask*/
        CHECK(err, sigprocmask(SIG_BLOCK, &mask, NULL), "sigprocmask");

        /*Aspetto che arrivi uno dei segnali settati dentro mask*/
        sigwait(&mask, &sig);
        
        switch (sig){
            case SIGINT:
                serveronline=0;
                printf("%d\n", serveronline);
                break;
            case SIGTERM:
                serveronline=0;
                break;
            case SIGUSR1:
                s_print_report();
                CHECK(err, sigaddset(&mask, SIGINT), "sigaddset");
                CHECK(err, sigaddset(&mask, SIGTERM), "sigaddset");
                CHECK(err, sigaddset(&mask, SIGUSR1), "sigaddset");
                break;
        }
    }
    pthread_exit(NULL);
}

void create_signal_t(){
    sigset_t mask;
    int err;
    /*Imposto a 1 le posizioni della mashera dei segnali*/
    CHECK(err, sigfillset(&mask), "sigfillset");

    /*Posso utilizzare siprocmask perché lavoro sul processo del server*/
    CHECK(err, sigprocmask(SIG_BLOCK, &mask, NULL), "sigprocmask");

    pthread_t os_signaller;
    CHECK(err, pthread_create(&os_signaller, NULL, &signaller, NULL), "pthread_create");
    CHECK(err, pthread_detach(os_signaller), "pthread_detach")
}