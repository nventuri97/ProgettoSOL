#define _POSIX_C_SOURCE 200809L
#include<sys/wait.h>
#include<sys/uio.h>
#include<sys/socket.h>
#include<fcntl.h>

#include"util.h"
#include"access.h"

#define test_string "Lo sai per un gol io darei la vita... la mia vita Che in fondo lo so sara' una partita... infinita.."

static int success, failure, tot_test;
char *trainingData[20];

void training_data();
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

    char name[nl];
    strcpy(name,argv[1]);

    int err;
    /*Provo a connettermi al server per 5 volte, con 5 scelta arbitraria, in caso di mancata connessione riporto l'errore */
    CHECK(err, os_connect(name), "os_connect");
    char *end;
    long int action=strtol(argv[2], &end, 10);

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

void training_data(){
    size_t base_size=strlen(test_string);

    /*Preparo il primo elemento per i test*/
    trainingData[0]=(char*) calloc(base_size+1, sizeof(char));
    sprintf(trainingData[0], "%s", test_string);
    trainingData[0][base_size]='\0';

    int i;
    for(i=1;i<20;i++){
        /*costruisco la dimensione del blocco i incrementando e arrivano fino a circa 100k*/
        size_t i_size=i*100*52.65;
        trainingData[i]=(char*) calloc(i_size+1, sizeof(char));
        /*inizializzo la riga del buffer con \0*/
        memset(trainingData[i], '\0', i_size);

        for(size_t j = 0; j < i_size/base_size; j++) 
            sprintf(trainingData[i],"%s%s",trainingData[i],test_string);
    }
}

void store_test(){
    /*preparo i dati da salvare sul server*/
    training_data();

    /*Avvio un ciclo che mi fa la store dei 20 dati che ho generato con training_data()*/
    for(int i=0;i<20;i++){
        char filename[MAXNAME];
        sprintf(filename, "t-%d", i+1);
        size_t i_size=strlen(trainingData[i]);

        /*Adesso mando la store per memorizzare i dati*/
        int err;
        CHECK(err, os_store(filename, (void *) trainingData[i], i_size), "os_store");

        if(err==True){
            fprintf(stdout, "Dati salvati correttamente\n");
            success++;
        } else {
            fprintf(stderr, "Dati non salvati, qualcosa è andato storto\n");
            failure++;
        }
        tot_test++;
    }
}

void retrieve_test(){
    /*preparo i dati per controllare l'autenticità di quelli restituiti*/
    training_data();

    /*Eseguo la retrive di tutti i dati inseriti dal client*/
    for(int i=0;i<20;i++){
        char filename[MAXNAME];
        memset(filename, 0, MAXNAME);
        sprintf(filename, "t-%d", i+1);

        /*Eseguo la retrive*/
        void *data=os_retrieve(filename);
        if(strcmp(data, trainingData[i])==0){
            fprintf(stdout, "Recupero dati riuscito\n");
            success++;
        } else {
            fprintf(stderr, "Recupero dati fallito\n");
            failure++;
        }
        tot_test++;
    }
}

void delete_test(){
    /*Inizializzo direttamente i nomi dei file e mando la delete*/
    for(int i=0;i<20;i++){
        char filename[MAXNAME];
        int err;
        CHECK(err, sprintf(filename, "t-%d", i+1),"sprintf");

        /*Elimino i dati */
        CHECK(err, os_delete(filename), "os_delete");
        if(err==True){
            fprintf(stdout, "Dati rimossi correttamente\n");
            success++;
        } else {
            fprintf(stderr, "Dati non rimossi, qualcosa è andato storto\n");
            failure++;
        }
        tot_test++;
    }
}