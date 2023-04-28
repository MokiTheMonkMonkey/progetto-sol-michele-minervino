#include "./../includes/util.h"
#include "../includes/threadsPool.h"
#include <values.h>
#include <util.h>
#include <threadsPool.h>

void * sender(void * err) {

    int fd_sock ,* e;

    e = err;

    ec_meno1_c(fd_sock = socket ( AF_UNIX , SOCK_STREAM , 0 ) , "errore creazione socket:" , return err )

    struct sockaddr_un sa;

    struct timespec wait;
    wait.tv_nsec = 1000000000;
    wait.tv_sec = 1;

    sa.sun_family = AF_UNIX;
    strncpy( sa.sun_path , SOCK_NAME , SOCK_NAME_LEN );
    sa.sun_path[SOCK_NAME_LEN] = '\0';
    for(int i = 0; i < 10 ; i++){

        errno = 0;
        if(connect(fd_sock , (struct sockaddr *)&sa , 108) == 0){

            break;

        }

        if(errno != ENOENT){

            *e = errno;
            return e;

        } else{

            if(nanosleep(&wait, NULL) == -1){

                perror("nanosleep :");
                return e;

            }

        }

    }


    Mes * to_send = NULL;
    size_t w_bites  =0;

    while(1){


        to_send = popListMes ();

        if(to_send) {


            if(!strncmp(to_send -> nome , "quit" , 4 )){

                w_bites = -2;
                if(writen(fd_sock , &w_bites , sizeof(size_t)) == -1){

                    perror("write quit :");
                    return e;

                }
                free(to_send -> nome);
                free(to_send);
                return NULL;

            }

            w_bites = strnlen(to_send->nome, MAX_NAME) + 1;

            if(writen ( fd_sock , &w_bites , sizeof(size_t)) == -1){

                perror("srittura numero bytes :");
                return e;

            }

            if(writen( fd_sock , to_send -> nome , w_bites ) == -1){

                perror( "scrittura messaggio :");
                return e;

            }
            if(writen ( fd_sock , &(to_send -> val) , sizeof(long int)) == -1){

                perror( "scrittura valore" );
                return e;

            }

            free(to_send -> nome);
            free(to_send);

        }


    }


}

void insertCoda(Nodo_Lista_Mes **lista,Nodo_Lista_Mes **last,Nodo_Lista_Mes  * Ins){

    Ins -> next = NULL;

    LOCK(&mes_list_mutex)


    //caso base, la lista e' vuota
    if(*lista == NULL){

        *lista = Ins;
        *last = *lista;

        SIGNAL(&mes_list_cond)
        UNLOCK(&mes_list_mutex)
        return;


    }
    //c'e' almeno un nodo
    (*last) -> next = Ins;
    (*last) = (*last) -> next;

    if(!strncmp(Ins -> msg -> nome , "quit" , 4)){

        end_list = 1;

    }

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

}


void init_coda_con(){

    coda_concorrente.th_number = 0;
    coda_concorrente.delay = _malloc(sizeof(struct timespec));
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

    if(coda_concorrente.delay -> tv_sec == 0) {

        coda_concorrente.delay -> tv_sec = 0;
        coda_concorrente.delay -> tv_nsec = 0;

    }
    if(coda_concorrente.lim == 0)

        coda_concorrente.lim = 8;

    is_set_coda_cond = 1;
    terMes = coda_concorrente.th_number;


}

int insert_coda_con(char * nomeFile){


    LOCK(&coda_mutex)

    if(nanosleep(coda_concorrente.delay, NULL ) == -1){

        perror( "errore nella nanosleep" );
        return -1;

    }

    while(!is_set_coda_cond && coda_concorrente.curr == coda_concorrente.lim){

        //coda troppo piena aspetto che si riempia
        WAIT(&coda_cond,&coda_mutex)

    }


    if(!coda_concorrente.coda){


        //non ci sono nodi quindi inserisco il primo
        coda_concorrente.coda = _malloc(sizeof(NodoCoda));
        coda_concorrente.coda -> dim =(int) strnlen ( nomeFile , MAX_NAME) + 1;
        coda_concorrente.coda -> nome = _malloc(coda_concorrente.coda -> dim);
        strncpy(coda_concorrente.coda -> nome , nomeFile , coda_concorrente.coda -> dim);
        coda_concorrente.coda -> next = NULL;
        coda_concorrente.last = coda_concorrente.coda;

    }
    else{

        //ci sono nodi quindi appendo all'ultimo
        NodoCoda * nuovoNodo = NULL;

        nuovoNodo = _malloc(sizeof(NodoCoda));
        nuovoNodo -> dim =(int) strnlen ( nomeFile , MAX_NAME) + 1;
        nuovoNodo -> nome = _malloc((nuovoNodo) -> dim);
        nuovoNodo -> next = NULL;
        strncpy((nuovoNodo) -> nome , nomeFile , (nuovoNodo) -> dim);
        coda_concorrente.last -> next = nuovoNodo;
        coda_concorrente.last = nuovoNodo;

    }
    //aggiorno la quantità di nodi presenti
    coda_concorrente.curr++;

    SIGNAL(&coda_cond)

    UNLOCK(&coda_mutex)

    return 0;

}

char * pop_Coda_Con(){

    NodoCoda * coda_next = NULL;

    LOCK(&coda_mutex)


    //aspetto che la coda si riempia o che arrivi il messaggio di teminazione
    while(!coda_concorrente.curr && !no_more_files){

        WAIT(&coda_cond,&coda_mutex)

    }

    //se il messaggio di terminazione e' arrivato ritorno null
    if(no_more_files && !coda_concorrente.curr){

        BCAST(&coda_cond)
        UNLOCK(&coda_mutex)
        return NULL;

    }


    char * fileName = _malloc(coda_concorrente.coda -> dim);

    if(--coda_concorrente.curr == 0){

        (coda_concorrente.last) = NULL;

    }
    else{

        coda_next = (coda_concorrente.coda) -> next;

    }


    strncpy(fileName,coda_concorrente.coda -> nome,coda_concorrente.coda -> dim);


    free((coda_concorrente.coda) -> nome);
    free(coda_concorrente.coda);

    (coda_concorrente.coda) = coda_next;

    if(!strncmp(fileName , "quit" , 4)){

        free(fileName);
        fileName = NULL;
        no_more_files = 1;

        BCAST( &coda_cond )
        UNLOCK(&coda_mutex)
        return NULL;

    }

    SIGNAL( &coda_cond )
    UNLOCK( &coda_mutex )

    return fileName;

}

int worker_Fun(void* filepath){

    //dichiaro le variabili
    char * filePath = (char*) filepath;

    if(!strncmp(filePath,"quit",4)){

        return 0;

    }
    FILE * fd = NULL;
    long int retValue = 0;
    int lBuf,i = 0;
    size_t filePathLen = strnlen(filePath , MAX_NAME) + 1;

    //apro il file
    if((fd = fopen (filePath , "r")) == NULL){

        perror("fopen :");
        return -1;

    }

    //calcolo il valore
    while(!feof(fd) && fread (&lBuf , sizeof (long) ,1,fd) != sizeof(long)){

        retValue += lBuf * i;
        i++;

    }

    //se si è interrotto per motivi diversi da EOF
    if(!feof (fd)){

        perror ("thread fread :");
        exit (1);

    }

    //chiudo il file
    errno = 0;

    if(fclose (fd) == EOF){

        int err = errno;
        perror("fclose :");
        exit(err);

    }


    //creo il nuovo nodo
    Nodo_Lista_Mes * nuovo = NULL;
    nuovo = _malloc (sizeof (NodoCoda));
    nuovo -> msg = _malloc(sizeof(Mes));
    nuovo -> msg -> nome = _malloc (filePathLen);
    strncpy (nuovo -> msg -> nome , filePath , filePathLen);
    nuovo -> msg -> val = retValue;
    free(filepath);

    //inserisco il nodo in lista
    insertCoda (&l_Proc_Ptr , &last_Proc_Ptr , nuovo);

    return 0;

}



void * worker(void * e){


    char* nomeFile;

    while(1) {

        if ((nomeFile = pop_Coda_Con ()) == NULL) {

            Nodo_Lista_Mes * ultimo = NULL;
            ultimo = _malloc (sizeof (NodoCoda));
            ultimo -> msg = _malloc (sizeof(Mes));
            ultimo -> msg -> nome = _malloc(5 * sizeof(char));
            strncpy (ultimo -> msg -> nome , "quit" , 5);
            ultimo -> msg -> val = MAXLONG;

            insertCoda (&l_Proc_Ptr , &last_Proc_Ptr ,  ultimo );

            return NULL;

        }

        if(worker_Fun(nomeFile) != 0){

            return e;

        }

    }
}


void printListCoda (NodoCoda *lptr){

    //funzione per il debugging stampa una lista
    if (!lptr){

        fprintf(stderr , "=================\n");
        return;

    }
    fprintf (stderr , "%d %s\n|\nV\n", lptr -> dim , lptr -> nome);
    printListCoda (lptr -> next);

}

Mes * popListMes (){

    LOCK(&mes_list_mutex)

    while (!(l_Proc_Ptr) && !end_list){

        WAIT ( &mes_list_cond , &mes_list_mutex )

    }

    if(!l_Proc_Ptr){

        if(terMes >= 0){

            terMes--;
            SIGNAL( &mes_list_cond )
            UNLOCK ( &mes_list_mutex )

            return NULL;

        }
        else{

            Mes * ret = _malloc(sizeof(Mes));
            ret -> nome = _malloc(sizeof(char) * 5);
            strncpy(ret -> nome , "quit" , 5);
            ret -> val = -2;

            SIGNAL ( &mes_list_cond )
            UNLOCK ( &mes_list_mutex )

            return ret;

        }

    }

    if(!strncmp( l_Proc_Ptr -> msg -> nome , "quit" , 4 )){

        terMes--;
        free(l_Proc_Ptr -> msg -> nome);
        free(l_Proc_Ptr -> msg);
        l_Proc_Ptr = NULL;

        SIGNAL(&mes_list_cond)
        UNLOCK(&mes_list_mutex)

        return NULL;

    }
    Nodo_Lista_Mes * next = (l_Proc_Ptr) -> next;

    size_t len = strnlen(l_Proc_Ptr -> msg -> nome, MAX_NAME) + 1;
    Mes * ret = _malloc(sizeof(Mes));
    ret -> nome = _malloc(len);
    strncpy( ret -> nome , (l_Proc_Ptr) -> msg -> nome , len  );
    ret -> val = (l_Proc_Ptr) -> msg -> val;

    free((l_Proc_Ptr) -> msg -> nome);
    free( (l_Proc_Ptr) -> msg);
    free(l_Proc_Ptr);

    (l_Proc_Ptr) = next;
    if(next == NULL){

        (last_Proc_Ptr) = NULL;

    }

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

    return ret;

}
