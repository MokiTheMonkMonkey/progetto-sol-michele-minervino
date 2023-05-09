#include <util.h>
#include <threadsPool.h>
#include <collector.h>
#include <bst.h>
#include <masterWorker.h>
#include "./../includes/util.h"
#include "./../includes/collector.h"
#include "./../includes/bst.h"
#include "./../includes/masterWorker.h"

#include <getopt.h>
#include <dirent.h>

//variabili globali

int is_set_coda_cond = 0 ,end_list = 0, no_more_files = 0;
long terMes;

sigset_t mask;

volatile sig_atomic_t  signExit = 0,printM = 0;

CodaCon coda_concorrente;
Nodo_Lista_Mes * l_Proc_Ptr = NULL;
Nodo_Lista_Mes * last_Proc_Ptr = NULL;
TreeNode * tree = NULL;

pthread_mutex_t ter_mes_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mes_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t coda_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t mes_list_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t coda_cond = PTHREAD_COND_INITIALIZER;

void cerca_File_Regolari( char * dirName ){


    if(dirName == NULL){

        return;

    }
    struct stat d_stat;
    DIR * dir = NULL;
    errno = 0;
    if((dir = opendir(dirName)) == NULL && !terMes){

        if( errno != 0){

            fprintf(stderr,"errore opendir : %s\n",dirName);
            return;


        }

    }

    struct dirent * info;
    char * file_name = NULL;
    while( ( errno = 0 ) , (( info = readdir(dir)) != NULL && !signExit) ){

        if( !( file_name = valid_name ( dirName , info -> d_name ) ) ){

            fprintf( stderr, "nome torppo lungo nella directory : %s" , dirName );

        }
        else if(stat(file_name , &d_stat) == -1){

            fprintf(stderr,"errore nella stat nel file :%s",file_name);
            free(file_name);
            exit(2);

        }
        else if((strncmp( info -> d_name , "." , 1) != 0) && (strncmp( info -> d_name , ".." , 2)) != 0){


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

char * ins_file_singoli( int argc , char * argv[] , int OptInd ){


    struct stat c_stat;
    while(OptInd < argc){

        if(strnlen(argv[OptInd],MAX_NAME) == MAX_NAME && argv[OptInd][MAX_NAME] != '\0'){

            fprintf( stderr , "il nome del file %s supera il limite di 255 caratteri :" , argv[OptInd++]);

        }

        if( (stat(argv[OptInd] , &c_stat) ) == -1 ){

            fprintf( stderr, "errore nel file :%s\n" , argv[OptInd++] );

        }
        else {
            if (S_ISREG(c_stat.st_mode)) {

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


int main (int argc , char* argv[]){

    if(argc < 2){

        perror("inserire argomenti :");
        exit(1);

    }

    //vari flag per le opzioni
    char nCase = 0 , qCase = 0;
    int d_Case = 0, tCase = 0 , option;

    char * dir_name = NULL;

    //inizializzo la coda concorrente
    init_coda_con();

    opterr = 0;

    while((option = getopt(argc,argv,":d:t:n:q:")) != -1){

        switch(option) {

            case 't':

                ISSET_CODA(tCase, "delay inseribile una sola volta\n")
                MU_ZERO((tCase = isNumber (optarg )) , "inserire un delay positivo")

                //converto il tCase in secondi
                coda_concorrente.delay -> tv_sec = tCase / 1000;
                //converto il tCase da millisecondi a nanosecondi
                coda_concorrente.delay -> tv_nsec = (tCase % 1000) * 1000000;
                tCase = 1;

                break;

            case 'n':

                ISSET_CODA(nCase , "numero di threads inseribile un sola volta\n")
                nCase = 1;
                MU_ZERO((coda_concorrente.th_number = isNumber(optarg)), "inserire un numero di thread maggiore di zero" )

                break;

            case 'q':

                ISSET_CODA(qCase, "limite coda concorrente inseribile una sola volta\n")
                qCase = 1;
                MU_ZERO((coda_concorrente.lim = isNumber(optarg)) , "inserire un limite queque maggiore di zero" )

                break;

            case 'd':

                ISSET_CODA(d_Case , "inseririe solo una cartella da analizzare\n" )
                d_Case = 1;
                size_t dirLen = strnlen(optarg,MAX_NAME) + 1;
                dir_name = s_malloc(dirLen);
                strncpy(dir_name,optarg,dirLen);

                break;

            case ':':
            case '?':
            default:

                fprintf(stderr,"inserie opzione valida\n");
                tutorial()
                exit( 1 );

        }

    }

    //non sono stati inseriti file o cartelle tra gli argomenti
    if(optind == argc && !d_Case){

        fprintf( stderr, "nessuna cartella o file da analizzare.\n" );
        tutorial()
        exit ( 1 );

    }

    int pid;
    /*
     * creo il processo collector
     *
     */
    IS_MENO1(pid = fork() , "errore fork :" , exit (1 ) )

    if(pid == 0){

        /*
         * e' il figlio
         *collector
         * */

        signalMask();

        //libero la memoria allocata nel vecchio processo
        free(dir_name);
        free(coda_concorrente.delay);

        atexit(&collectorExitFun);

        int r_sock;
        size_t r_bites = 0;
        Mes message;

        if((r_sock = sock_connect()) == -1 ){

            perror("Master connect ");
            return -1;

        }

        while(1) {


            //leggo la lunghezza della stringa da leggere o eventuali segnali
            if (read(r_sock, &r_bites, sizeof(size_t)) == -1) {

                return -1;

            }
            //messaggio di terminazione
            if (r_bites == -2)

                break;

            //richiesta di stampa
            if(r_bites == -3){

                printTree(tree);

            }
            else {


                if (r_bites <= 0)

                    return -1;

                //alloco spazio per il nome del file
                message.nome = s_malloc(r_bites);

                if (readn(r_sock, message.nome, r_bites) == -1) {


                    return -1;

                }

                if (!strncmp(message.nome, "quit", 4)) {


                    break;

                }

                if (readn(r_sock, &(message.val), sizeof(long int)) == -1) {

                    fprintf(stderr, "valore");
                    exit(3);
                }

                insTree(message, &tree);
                free(message.nome);

            }

        }
        IS_MENO1(close(r_sock) , "close :" , return -1)
        IS_MENO1(unlink(SOCK_NAME) , "unlink :" , return -1)
        printTree(tree);

        return 0;

    }

    /*
     * processo padre
     * master Worker
     * */


    pthread_t send,signalH;

    atexit(&masterExitFun);

    signalMask();

    pthread_create(&signalH,NULL, signalHandler,NULL);

    //inizializzo ai valori standard le opzioni non scelte e mando un segnale al' thread addetto lla ricerca
    set_standard_coda_con();

    int e = 0;



    /*
     * faccio partire i threads prima di mettere in coda i file singoli
     * cosi' posso far farli concorrere durante l'inserimento
     * */

    for(int i = 0; i < coda_concorrente.th_number; i++) {


        NOT_ZERO(pthread_create(&(coda_concorrente.workers[i]), NULL, worker, NULL ) , "errore creazione worker")

    }

    NOT_ZERO(pthread_create( &send , NULL , sender , (void *)&e) , "errore creazione sender")


    if(optind != argc){

        ins_file_singoli( argc , argv , optind );

    }

    //è stata analizzata una cartella e controllo se non ci sono stati errori
    if(d_Case){

        cerca_File_Regolari ( dir_name );
        free(dir_name);

    }

    insert_coda_con("quit");

    for(int i = 0; i < coda_concorrente.th_number ; i++){

        if(pthread_join( coda_concorrente.workers[i] , NULL ) != 0){

            //gestione errori e free
            perror("join worker ");
            return -1;

        }


    }


    if(pthread_join( send , NULL ) != 0){

        //gestione errori e free
        exit(2);

    }

    if(waitpid(pid, NULL , 0) == -1){

        perror("errore nella WAIT PID :");
        exit(e);

    }

    if((e = pthread_kill(signalH,SIGQUIT)) != 0){

        perror("kill signal handler ");
        exit(e);

    }

    if(pthread_join(signalH,NULL) != 0){

        perror("join signal handler ");
        exit(EXIT_FAILURE);

    }

    return 0;

}
