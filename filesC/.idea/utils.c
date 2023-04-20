#include <utils.h>

void* _malloc (unsigned long size) {

    void *elem;

    if ((elem = malloc(size)) == NULL && size != 0){

        perror("malloc :");
        exit(EXIT_FAILURE);

    }

    return elem;

}


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

void insert_coda_con(char * nomeFile){

    LOCK(&coda_mutex)

    while(coda_concorrente.curr == coda_concorrente.lim){

        //coda troppo piena aspetto che si riempia
        WAIT(&insert_coda_cond,&coda_mutex)

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

}

int isNumber(const char* s) {

    char* e = NULL;
    int val = (int)strtol(s, &e, 0);
    if (e != NULL && *e == (char)0) return val;
    return -1;

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

char * pop_Coda_Con(){

    NodoCoda * coda_next = NULL;

    LOCK(&coda_mutex)

    //aspetto che la coda si riempia o che arrivi il messaggio di teminazione
    while(!coda_concorrente.curr && !cond_Master){

        WAIT(&coda_cond,&coda_mutex)

    }

    //se il messaggio di terminazione e' arrivato ritorno null
    if(cond_Master){

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
        cond_Master = 1;
        BCAST( &coda_cond )

    }
    SIGNAL( &insert_coda_cond )
    UNLOCK( &coda_mutex )

    return fileName;

}


void worker_Fun(void* filepath){

    //dichiaro le variabili
    char * filePath = (char*) filepath;
    FILE * fd = NULL;
    long int retValue = 0;
    long lBuf = 0 , i = 0;
    unsigned long filePathLen = strnlen(filePath , MAX_NAME) + 1;

    //apro il file
    ec_null(fd = fopen (filePath , "r") , "thread fopen :")

    //calcolo il valore
    while(fread (&lBuf , sizeof (long) ,1,fd) != 0){

        retValue += lBuf * i++;

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

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

}


void * worker(){

    char* nomeFile;
    while(1) {


        if ((nomeFile = pop_Coda_Con ()) == NULL) {

            return NULL;

        }

        worker_Fun(nomeFile);


    }
}

void *genera(void *argv){

    char **Argv = (char**)argv;
    int i = 1;
    while(Argv[i]) {

        insert_coda_con(Argv[i]);
        i++;

    }

    insert_coda_con("quit");

    return NULL;

}