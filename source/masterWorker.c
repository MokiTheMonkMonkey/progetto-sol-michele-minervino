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

    is_set_coda_con = 1;
    terMes = coda_concorrente.th_number;

}


/*
 * ricerca ricorsivamente e inserisce file regolari dalla cartella
 * */
void cerca_File_Regolari( char * dirName ){

    if(dirName == NULL){

        return;

    }
    struct stat d_stat;
    DIR * dir = NULL;

    errno = 0;
    //apro la cartella
    if((dir = opendir(dirName)) == NULL || signExit ){


        if( errno != 0){

            fprintf(stderr,"errore opendir : %s\n",dirName);
            return;


        }

        if(signExit){

            if(closedir(dir) != 0){

                fprintf(stderr,"errore closedir : %s\n",dirName);

            }

            return;

        }


    }

    struct dirent * info;
    char * file_name = NULL;

    //finche' la cartella non è vuota,si veirfica un errore o non viene mandato il segnale SIGINT
    while( ( errno = 0 ) , (( info = readdir(dir)) != NULL && !signExit) ){

        if( !( file_name = valid_name ( dirName , info -> d_name ) ) ){

            fprintf( stderr, "nome torppo lungo nella directory : %s" , dirName );

        }
        else if(stat(file_name , &d_stat) == -1){

            fprintf(stderr,"errore nella stat nel file :%s",file_name);
            free(file_name);
            exit(2);

        }
        else if((strncmp( info -> d_name , "." , 1) != 0) && (strncmp( info -> d_name , ".." , 2)) != 0){//controllo che non siano le cartelle "." o ".."

            //controllo se il file e' regolare
            if (S_ISREG(d_stat.st_mode)) {

                insert_coda_con( file_name );

                if( nanosleep( coda_concorrente.delay , NULL ) != 0 ){

                    perror ("nanosleep :");
                    exit ( EXIT_FAILURE );

                }


            } else {

                if ( S_ISDIR ( d_stat.st_mode ) && !signExit) {


                    cerca_File_Regolari ( file_name );

                }
                else{

                    if(!signExit)

                        fprintf(stderr , "nella cartella %s torvato file non regolare :%s\n" , dirName , file_name);

                }


            }


        }

        free(file_name);

    }

    errno = 0;
    if(closedir( dir ) != 0 ) {

        perror("closedir :");
        exit(errno);

    }

}

/*
 * funzione che inserisce i file da argv
 * */
char * ins_file_singoli( int argc , char * argv[] , int OptInd ){


    struct stat c_stat;
    while( !signExit && OptInd < argc  ){

        if(strnlen(argv[OptInd],MAX_NAME) == MAX_NAME && argv[OptInd][MAX_NAME] != '\0'){

            fprintf( stderr , "il nome del file %s supera il limite di 255 caratteri :" , argv[OptInd++]);

        }

        if( (stat(argv[OptInd] , &c_stat) ) == -1 ){

            fprintf( stderr, "errore nel file :%s\n" , argv[OptInd++] );

        }
        else {
            if (S_ISREG(c_stat.st_mode) && !signExit) {

                insert_coda_con(argv[OptInd++]);

                if (nanosleep(coda_concorrente.delay, NULL) != 0) {

                    perror("nanosleep : ");
                    exit(EXIT_FAILURE);

                }

            } else {

                fprintf(stderr, "errore nel file :%s\n", argv[OptInd++]);

            }

        }
    }

    return NULL;

}

