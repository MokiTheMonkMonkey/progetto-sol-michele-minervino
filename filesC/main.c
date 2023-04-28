#include <util.h>
#include <threadsPool.h>
#include <collector.h>
#include <bst.h>
#include "./../includes/bst.h"
#include "../includes/util.h"
#include "../includes/threadsPool.h"
#include "./../includes/collector.h"
#include <getopt.h>
#include <dirent.h>

//variabili globali
int is_set_coda_cond = 0 ,end_list = 0, no_more_files = 0;
long terMes;
char dCase = 0;
CodaCon coda_concorrente;
Nodo_Lista_Mes * l_Proc_Ptr = NULL;
Nodo_Lista_Mes * last_Proc_Ptr = NULL;


pthread_mutex_t mes_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mes_list_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t coda_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t coda_cond = PTHREAD_COND_INITIALIZER;


void * cerca_File_Regolari( void * dir_Name ){

    char * dirName = (char *) dir_Name;
    struct stat d_stat;
    DIR * dir;
    errno = 0;
    dir = opendir(dirName);
    if( errno != 0){


        fprintf(stderr , "%s",dirName);
        perror("errore opendir");
        return dir_Name;


    }
    struct dirent * info;
    char * file_name;
    while((errno = 0), ( info = readdir(dir)) != NULL ){

        if(!(file_name = valid_name(dirName,info -> d_name))){

            perror( "nome torppo lungo" );
            return dir_Name;

        }

        if(stat(file_name , &d_stat) == -1){

            perror("stat :");
            return dir_Name;

        }

        if((strncmp( info -> d_name , "." , 1) != 0) && (strncmp( info -> d_name , ".." , 2)) != 0){


            if (S_ISREG(d_stat.st_mode)) {

                insert_coda_con(file_name);


            } else {

                if (S_ISDIR(d_stat.st_mode)) {


                    if (!(file_name = valid_name(dirName, info->d_name))) {

                        perror("nome troppo lungo");
                        return dir_Name;

                    }
                    if (cerca_File_Regolari(file_name)) {

                        return dir_Name;

                    }

                }

            }

        }
        free(file_name);

    }

    return NULL;

}

int ins_file_singoli( int argc , char * argv[] , int OptInd ){

    struct stat c_stat;
    while(OptInd < argc){

        if( (stat(argv[OptInd] , &c_stat) ) == -1 ){

            perror("errore nella stat :");
            return -1;

        }
        if(S_ISREG( c_stat.st_mode)){

            insert_coda_con(argv[OptInd++]);

        }
        else{

            return -1;

        }


    }

    return 0;

}


int main (int argc , char* argv[]){

    if(argc < 2){

        perror("allora :");
        exit(1);

    }

    long delay = 0;
    int option;
    char eW = 1;
    char * dir_name = NULL;

    int pid;



    init_coda_con();

    while((option = getopt(argc,argv,"d:t:n:q:")) != -1){

        switch(option) {

            case 't':

                ISSET_CODA(delay, "delay inseribile una sola volta")
                ec_min_zero(delay = isNumber(optarg),"inserire un delay positivo")

                //converto il delay in secondi
                coda_concorrente.delay -> tv_sec = delay/1000;
                //converto il delay da millisecondi a nanosecondi
                coda_concorrente.delay -> tv_nsec = (delay%1000)*1000000;

                break;

            case 'n':

                ISSET_CODA(coda_concorrente.th_number, "numero di threads inseribile un sola volta")
                ec_mu_zero((coda_concorrente.th_number = isNumber(optarg)),"inserire un numero di thread maggiore di zero" )
                break;

            case 'q':

                ISSET_CODA(coda_concorrente.lim, "limite coda concorrente inseribile una sola volta")
                ec_mu_zero((coda_concorrente.lim = isNumber(optarg)) , "inserire un limite queque maggiore di zero" )

                break;

            case 'd':

                ISSET_CODA( dCase , "inseririe solo una cartella da analizzare" )
                dCase = 1;
                size_t dirLen = strnlen(optarg,MAX_NAME) + 1;
                dir_name = _malloc(dirLen);
                strncpy(dir_name,optarg,dirLen);

                break;

            case '?':

                fprintf(stderr,"opzione %c non valida",option);
                return 1;

            default:

                fprintf(stderr,"inserie opzione valida");
                return 1;

        }

    }


    //non sono stati inseriti file o cartelle tra gli argomenti
    if(optind == argc && !dCase){

            perror("nessuna cartella o file da analizzare");
            return -1;



    }



/*
     * creo il processo collector
     * */
    ec_meno1_c( pid = fork() , "errore fork :" , return -1 )

    if(pid == 0){

        /*
         * e' il figlio
         *collector
         * */

        int r_sock;
        size_t r_bites = 0;
        Mes message;
        TreeNode * tree = NULL;

        if((r_sock = sock_connect()) == -1 ){


            //gestione errori con eventuale unlink

            return -1;

        }

        while(1) {


            if (read(r_sock, &r_bites, sizeof(size_t)) == -1) {


                //gestione errori con unlink e messaggio al master
                return -1;

            }
            if (r_bites == -2)

                break;

            if (r_bites <= 0)

                return -1;

            message.nome = _malloc(r_bites);
            if (readn(r_sock, message.nome, r_bites) == -1) {

                //gestione errorri con unlink messaggio al master

                return -1;

            }
            if (!strncmp(message.nome, "quit", 4)) {


                break;

            }
            if (read(r_sock, &(message.val), sizeof(long int)) == -1){

                fprintf(stderr, "valore");
                return -2;
            }
            insTree( message , &tree );
            free(message.nome);


        }
        ec_meno1_c(unlink(SOCK_NAME) , "unlink :" ,return -1)
        printTree(tree);
        freeTree(tree);
        return 0;

    }

    /*
     * processo padre
     * master Worker
     * */

    //inizializzo ai valori standard le opzioni non scelte e mando un segnale al' thread addetto lla ricerca
    set_standard_coda_con();

    int e = 0;
    pthread_t send;
    pthread_t * workers = _malloc(sizeof(pthread_t) * coda_concorrente.th_number);


    /*
     * faccio partire i threads prima di mettere in coda i file singoli
     * cosi' posso far farli concorrere durante l'inserimento
     * */

    for(int i = 0; i < coda_concorrente.th_number; i++) {


        NOT_ZERO(pthread_create(&(workers[i]), NULL, worker, (void*)&eW ) , "errore creazione worker")

    }

    NOT_ZERO(pthread_create( &send , NULL , sender , (void *)&e) , "errore creazione sender")


    if(optind != argc){


        if(ins_file_singoli( argc , argv , optind)){

            perror("non inserire file non regolari tra gli argomenti");
            //gestione errori con free
            return -1;

        }

    }

    //è stata analizzata una cartella e controllo se non ci sono stati errori
    if(dCase){

        if(cerca_File_Regolari(dir_name) != NULL){

            return -1;

        }
        free(dir_name);

    }
    insert_coda_con("quit");

    for(int i = 0; i < coda_concorrente.th_number ; i++){

        if(pthread_join( (workers[i]) , NULL ) != 0){

            //gestione errori e free
            return -1;

        }


    }

    if(pthread_join( send , NULL ) != 0){

        //gestione errori e free
        return -1;

    }

    free(workers);

    if(waitpid(pid, NULL , 0) == -1){

        perror("errore nella WAIT PID :");
        return -1;

    }

    return 0;

}
