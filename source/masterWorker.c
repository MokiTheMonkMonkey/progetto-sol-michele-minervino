#include <masterWorker.h>
#include "./../includes/masterWorker.h"


/*
 * signal handler
 * */
void *signalHandler(){

    int signum,err;

    while ( 1 ) {

        //aspetto i segnali
        err = sigwait(&mask, &signum);

        if(err != 0) {

            perror("sigwait ");        //sigwait smaschera e si sospende
            exit(EXIT_FAILURE);

        }

        //controllo qual' e' il segnale
        switch(signum) {

            case SIGHUP:
            case SIGINT:
            case SIGQUIT:
            case SIGTERM:

                signExit = 1;
                pthread_exit(NULL);

            case SIGUSR1:

                printM =1;
                break;

            default:
                ;

        }

    }

}

/*
 * funzione che maschera i segnali
 * */
int signalMask(){

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;

    //ignoro il SIGPIPE
    if(sigaction(SIGPIPE,&sa, NULL) == -1) {
        perror("[MASTERWORKER] sigaction SIGPIPE");
        return -1;
    }

    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);   // aggiunto SIGHUP alla maschera
    sigaddset(&mask, SIGINT);   // aggiunto SIGINT alla maschera
    sigaddset(&mask, SIGQUIT);  // aggiunto SIGQUIT alla maschera
    sigaddset(&mask, SIGTERM);  // aggiunto SIGTERM alla maschera
    sigaddset(&mask, SIGUSR1);  // aggiunto SIGUSR1 alla maschera
    //maschero i segnali
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {

        perror("[MASTERWORKER] pthread_sigmask");
        return -1;

    }

    return 0;

}

/*
 * funzione di terminazione del master
 * */
void masterExitFun(){

    free(coda_concorrente.delay);
    free(coda_concorrente.workers);

    NodoCoda * next = NULL;

    while(coda_concorrente.coda){

        next = coda_concorrente.coda -> next;
        free(coda_concorrente.coda -> nome);
        free(coda_concorrente.coda);
        coda_concorrente.coda = next;

    }


    Nodo_Lista_Mes * next_mes = NULL;

    while(Coda_Mes_ptr){

        next_mes = Coda_Mes_ptr -> next;
        free(Coda_Mes_ptr);
        Coda_Mes_ptr = next_mes;

    }

}


/*
 * inizializzazione coda concorrente
 * */
void init_coda_con(){

    coda_concorrente.th_number = 0;
    coda_concorrente.delay = s_malloc(sizeof(struct timespec));
    coda_concorrente.delay -> tv_sec = 0;
    coda_concorrente.delay -> tv_nsec = 0;
    coda_concorrente.lim = 0;
    coda_concorrente.curr = 0;
    coda_concorrente.coda = NULL;
    coda_concorrente.last = NULL;

}

/*
 * funzione che setta allo standard tutte le statistiche che non sono state settate
 * */
void set_standard_coda_con(){


    if(coda_concorrente.th_number == 0)

        coda_concorrente.th_number = 4;

    coda_concorrente.workers = s_malloc(sizeof(pthread_t) * coda_concorrente.th_number);


    if(coda_concorrente.lim == 0)

        coda_concorrente.lim = 8;

    is_set_coda_cond = 1;
    terMes = coda_concorrente.th_number;

}
