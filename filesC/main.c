#include <util.h>
#include <threadsPool.h>
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
    struct stat d_stat;
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

        if(stat(info -> d_name , &d_stat) == -1){


            return dir_Name;

        }
        if(S_ISREG( d_stat.st_mode )){

            if(!(file_name = valid_name(dirName,info -> d_name))){

                perror( "nome torppo lungo" );
                return dir_Name;

            }
            insert_coda_con( file_name );


        }
        else{

            if(S_ISDIR( d_stat.st_mode )){

                if(!(strncmp( info -> d_name , "." , 1)) || !(strncmp( info -> d_name , ".." , 2)))

                    break;

                if(!( file_name = valid_name( dirName , info -> d_name ) ) ){

                    perror( "nome troppo lungo" );
                    return dir_Name;

                }
                if(cerca_File_Regolari( file_name )){

                    return dir_Name;

                }

            }

        }
        /*switch ( info -> d_type ){

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

        */
    }

    return NULL;

}

int ins_file_singoli( char * argv[] , int OptInd ){

    struct stat c_stat;
    while(argv[OptInd] != NULL){

        if( (stat(argv[OptInd] , &c_stat) ) == -1 ){

            perror("errore nella stat :");
            return -1;

        }
        if(S_ISREG( c_stat.st_mode)){

            insert_coda_con(argv[optind++]);

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

    long delay = -1;
    int option;
    char dCase = 0, eW = 1;

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
                char * dir_name = _malloc(dirLen);
                strncpy(dir_name,optarg,dirLen);

                NOT_ZERO( pthread_create( &searcher , NULL , cerca_File_Regolari , (void *)dir_name ) , "pthread create : [searcher]" )

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
    if(optind == argc && dCase != 1 ){

        perror( "nessuna cartella o file da analizzare" );
        return -1;

    }

    //inizializzo ai valori standard le opzioni non scelte e mando un segnale al' thread addetto lla ricerca
    set_standard_coda_con();

    pthread_t * workers = _malloc(sizeof(pthread_t) * coda_concorrente.th_number);

    /*
     * faccio partire i threads prima di mettere in coda i file singoli
     * cosi' posso far farli concorrere durante l'inserimento
     * */
    for(int i = 0; i < coda_concorrente.th_number; i++) {

        NOT_ZERO(pthread_create(&(workers[i]), NULL, worker, (void*)&eW ) , "errore creazione worker")

    }
    if(optind != argc){

        if(ins_file_singoli(argv , optind)){

            perror("non inserire file non regolari tra gli argomenti");
            //gestione errori con free
            return -1;

        }
    }

    //è stata analizzata una cartella e controllo se non ci sono stati errori
    if(dCase){

        if(pthread_join(searcher , NULL ) != 0){

            //gestione errori con free
            return -1;

        }

    }

    for(int i = 0; i < coda_concorrente.th_number ; i++){

        if(pthread_join( workers[i] , NULL ) != 0){

            //gestione errori e free
            return -1;

        }

    }



    printList (l_Proc_Ptr);

    return 0;

}
