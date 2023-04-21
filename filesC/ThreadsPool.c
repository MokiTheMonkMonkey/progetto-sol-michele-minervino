#include "./../includes/util.h"
#include "../includes/threadsPool.h"
#include <util.h>
#include <threadsPool.h>

void insertCoda(Nodo_Lista_Mes **lista,Nodo_Lista_Mes **last,Nodo_Lista_Mes * ins){

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
    coda_concorrente.lim = 0;
    coda_concorrente.curr = 0;
    coda_concorrente.coda = NULL;
    coda_concorrente.last = NULL;

}

/*
 * funzione che setta allo standard tutte le statistiche che non sono state settate
 * */
void set_standard_coda_con(){


    LOCK( &coda_mutex )

    if(coda_concorrente.th_number == 0) coda_concorrente.th_number = 4;
    if(coda_concorrente.delay -> tv_sec == 0) {

        coda_concorrente.delay -> tv_sec = 0;
        coda_concorrente.delay -> tv_nsec = 0;

    }
    if(coda_concorrente.lim == 0) coda_concorrente.lim = 8;

    is_set_coda_cond = 1;

    BCAST( &coda_cond )

    UNLOCK( &coda_mutex )

}

int insert_coda_con(char * nomeFile){

    if(nanosleep(coda_concorrente.delay, NULL ) == -1){

        perror( "errore nella nanosleep" );
        return -1;

    }
    LOCK(&coda_mutex)


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

    BCAST(&coda_cond)

    UNLOCK(&coda_mutex)

    return 0;

}

char * pop_Coda_Con(){

    NodoCoda * coda_next = NULL;

    LOCK(&coda_mutex)

    //aspetto che la coda si riempia o che arrivi il messaggio di teminazione
    while(!coda_concorrente.curr && !no_more_files){

        fprintf(stderr, "we");
        WAIT(&coda_cond,&coda_mutex)

    }

    //se il messaggio di terminazione e' arrivato ritorno null
    if(no_more_files){

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

    }

    BCAST( &coda_cond )
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
    nuovo -> nome = _malloc (filePathLen);
    strncpy (nuovo -> nome , filePath , filePathLen);
    nuovo -> val = retValue;


    //prendo la lock e inserisco il nodo in lista
    LOCK(&mes_list_mutex)

    insertCoda (&l_Proc_Ptr , &last_Proc_Ptr , nuovo);
    printList(l_Proc_Ptr);

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

    return 0;

}


void * worker(void * e){


    char* nomeFile;

    while(1) {

        fprintf(stderr, ".");
        if ((nomeFile = pop_Coda_Con ()) == NULL) {


            return NULL;

        }
        fprintf(stderr, "eeeeh ehhh");
        if(worker_Fun(nomeFile) != 0){

            return e;

        }

    }
}

void printList (Nodo_Lista_Mes *lptr){

    //funzione per il debugging stampa una lista
    if (!lptr){

        fprintf(stderr,"=================\n");
        return;

    }
    fprintf (stderr , "%ld %s\n|\nV\n", lptr -> val , lptr -> nome);
    printList (lptr -> next);

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
