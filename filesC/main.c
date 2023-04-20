#include <util.h>
#include "../includes/util.h"
#include "../includes/threadsPool.h"
#include <getopt.h>
#include <dirent.h>

//variabili globali
int is_set_coda_cond = 0,cond_Master = 0;
CodaCon coda_concorrente;
Nodo_Lista_Mes * l_Proc_Ptr = NULL;
Nodo_Lista_Mes * last_Proc_Ptr = NULL;


pthread_mutex_t mes_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mes_list_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t coda_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t coda_cond = PTHREAD_COND_INITIALIZER;

char * valid_name(char * dirname , char * next){

    size_t len1,len2;

    len1 = strnlen(dirname , MAX_NAME);
    len2 = strnlen(next , MAX_NAME);

    if(len1 + len2 + 1 > MAX_NAME)

        return NULL;

    char * str_ret = _malloc(len1 + len2 +2);
    strncpy( str_ret , dirname , len1);
    str_ret[len1 +1] = '/';
    str_ret[len1 + 2] = '\0';

    str_ret = strncat( str_ret , next , len1 + len2 + 2);

    return str_ret;

}

void * cerca_File_Regolari( void * dir_Name ){

    char * dirName = (char *) dir_Name;
    DIR * dir;
    errno = 0;
    dir = opendir(dirName);
    if( errno != 0){

        perror("errore opendir");
        return dir_Name;

    }
    struct dirent * info;
    char * file_name;
    while((errno = 0), ( info = readdir(dir)) != NULL ){

        switch ( info -> d_type ){

            case DT_REG:

                if(!(file_name = valid_name(dirName,info -> d_name))){

                    perror( "nome torppo lungo" );
                    return dir_Name;

                }
                insert_coda_con( file_name );

                break;

            case DT_DIR:

                if(!(strncmp( info -> d_name , "." , 1)) || !(strncmp( info -> d_name , ".." , 2)))

                    break;

                if(!( file_name = valid_name( dirName , info -> d_name ) ) ){

                    perror( "nome troppo lungo" );
                    return dir_Name;

                }
                if(cerca_File_Regolari( file_name )){

                    return dir_Name;

                }

            default:

                break;


        }

    }

    return NULL;

}


int main (int argc , char* argv[]){

    if(argc < 2){

        perror("allora :");
        exit(1);

    }

    long delay = -1;
    int option;
    char dCase = -1;
    pthread_t searcher;
    init_coda_con();

    while((option = getopt(argc,argv,"d:t:n:q")) != -1){

        switch(option) {

            case 't':

                ISSET_CODA(delay, "delay inseribile una sola volta")
                ec_min_zero(delay = isNumber(optarg),"inserire un delay positivo")

                //converto il delay in secondi
                coda_concorrente.delay -> tv_sec = delay/1000;
                //converto il delay da millisecondi a nanosecondi
                coda_concorrente.delay -> tv_nsec = (delay%1000)*1000000;


            case 'n':

                ISSET_CODA(coda_concorrente.th_number, "numero di threads inseribile un sola volta")
                ec_mu_zero(coda_concorrente.th_number = isNumber(optarg),"inserire un numero di thread maggiore di zero" )
                break;

            case 'q':

                ISSET_CODA(coda_concorrente.lim, "limite coda concorrente inseribile una sola volta")
                ec_mu_zero(coda_concorrente.lim = isNumber(optarg) , "inserire un limite queque maggiore di zero" )
                break;

            case 'd':

                ISSET_CODA( dCase , "inseririe solo una cartella da analizzare" )
                dCase = 1;
                size_t dirLen = strnlen(optarg,MAX_NAME) + 1;
                s_Var * search_var = _malloc( sizeof( s_Var ) );
                search_var -> dir_name = _malloc(dirLen);
                search_var -> error = 0;
                strncpy(search_var -> dir_name,optarg,dirLen);

                NOT_ZERO( pthread_create( &searcher , NULL , cerca_File_Regolari , (void *)search_var ) , "pthread create : [searcher]" )



                break;

            case '?':

                fprintf(stderr,"opzione %c non valida",option);
                return 1;
                break;

            default:

                fprintf(stderr,"inserie opzione valida");
                return 1;
                break;

        }

    }

    //non sono stati inseriti file o cartelle tra gli argomenti
    if(optind == argc && dCase != 1 ){

        perror( "nessuna cartella o file da analizzare" );
        return -1;

    }

    set_standard_coda_con();


    //è stata analizzata una cartella e controllo se non ci sono stati errori
    if(dCase){

        if(pthread_join(searcher , NULL ) != 0){

            return -1;

        }

    }
    for(int i = 0; i < threads_number ; i++){

        NOT_ZERO(pthread_create (&worker_Threads[i] , NULL , worker , NULL),"pthread create:")

    }
    for(int i = 0 ; i < threads_number ; i++){

        NOT_ZERO(pthread_join (worker_Threads[i] , NULL),"pthread join")

    }

    printList (l_Proc_Ptr);

    return 0;

}
