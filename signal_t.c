#define _POSIX_C_SOURCE 200809L
#include"signal_t.h"

void s_print_report(){
    fprintf(stdout, "----------------SIGUSR1 catturato----------------\n");
    fprintf(stdout, "----------------Informazioni object store----------------\n");
    fprintf(stdout, "Client connessi: %d\n", conn_client);
    fprintf(stdout, "Oggetti memorizzati nell'object store: %d\n", n_obj);

    pthread_mutex_lock(&mtx);
    /*Converto la size dello store in Kb*/
    tot_size=tot_size/1024;
    pthread_mutex_unlock(&mtx);

    if(tot_size>=1024)
        fprintf(stdout, "Dimensione dell'objectstore: %f Mb\n",(double) tot_size/1024);
    else
        fprintf(stdout, "Dimensione dell'objectstore: %f Kb\n",(double) tot_size);
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
                fprintf(stdout, "----------------SIGINT ricevuto----------------\n");
                fprintf(stdout, "----------------Objectstore in chiusura----------------\n");
                break;
            case SIGTERM:
                serveronline=0;
                fprintf(stdout, "----------------SIGTERM ricevuto----------------\n");
                fprintf(stdout, "----------------Objectstore in chiusura----------------\n");
                break;
            case SIGUSR1:
                s_print_report();
                CHECK(err, sigaddset(&mask, SIGINT), "sigaddset");
                CHECK(err, sigaddset(&mask, SIGTERM), "sigaddset");
                CHECK(err, sigaddset(&mask, SIGUSR1), "sigaddset");
                pthread_mutex_lock(&mtx);
                conn_client=0;
                n_obj=0;
                tot_size=0;
                pthread_mutex_unlock(&mtx);
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