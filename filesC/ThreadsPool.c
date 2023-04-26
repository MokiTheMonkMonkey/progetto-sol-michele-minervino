#include "./../includes/util.h"
#include "../includes/threadsPool.h"
#include <values.h>
#include <util.h>
#include <threadsPool.h>

void insertCoda(Nodo_Lista_Mes **lista,Nodo_Lista_Mes **last,Nodo_Lista_Mes  * Ins){

    Nodo_Lista_Mes * ins = _malloc(sizeof(Nodo_Lista_Mes));
    ins -> next = NULL;
    //caso base, la lista e' vuota
    if(*lista == NULL){

        *lista = ins;
        *last = *lista;
        return;

    }

    //c'e' almeno un nodo
    (*last) -> next = ins;
    (*last) = (*last) -> next;

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
    if(no_more_files){

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



    }
    FILE * fd = NULL;
    long int retValue = 0;
    int lBuf;
    unsigned long filePathLen = strnlen(filePath , MAX_NAME) + 1;

    //apro il file
    if((fd = fopen (filePath , "r")) == NULL){

        perror("fopen :");
        return -1;

    }

    //calcolo il valore
    while(!feof(fd) && fread (&lBuf , sizeof (long) ,1,fd) != sizeof(long)){

        retValue ++;

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
    free(filepath);
    nuovo -> msg -> val = retValue;


    //prendo la lock e inserisco il nodo in lista
    LOCK(&mes_list_mutex)

    insertCoda (&l_Proc_Ptr , &last_Proc_Ptr , nuovo);

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

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

            LOCK(&mes_list_mutex)

            insertCoda (&l_Proc_Ptr , &last_Proc_Ptr ,  ultimo );

            SIGNAL(&mes_list_cond)
            UNLOCK(&mes_list_mutex)

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

Mes * popListMes (Nodo_Lista_Mes ** lPtr, Nodo_Lista_Mes ** last){

    LOCK(&mes_list_mutex)

    while (!*lPtr && !end_list){

        WAIT ( &mes_list_cond , &mes_list_mutex )

    }

    Nodo_Lista_Mes * next = (*lPtr) -> next;

    Mes * ret = (*lPtr) -> msg;

    free(*lPtr);
    (*lPtr) = next;
    if(next == NULL){

        (last) = NULL;

    }

    UNLOCK(&mes_list_mutex)

    return ret;

}
