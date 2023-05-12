#include <threadsPool.h>


/*
 * funzione per inserire in coda
 * */
int insert_coda_con(char * nomeFile){


    LOCK(&coda_mutex)

    while(!is_set_coda_con && coda_concorrente.curr == coda_concorrente.lim && !signExit){

        //coda troppo piena aspetto che si svuoti
        WAIT(&coda_cond,&coda_mutex)

    }

    if(signExit){

        no_more_files = 1;
        SIGNAL(&coda_cond)
        UNLOCK(&coda_mutex)

        return 0;

    }

    if(!coda_concorrente.coda){


        //non ci sono nodi quindi inserisco il primo
        coda_concorrente.coda = s_malloc(sizeof(NodoCoda));
        coda_concorrente.coda -> dim = strnlen ( nomeFile , MAX_NAME) + 1;
        coda_concorrente.coda -> nome = s_malloc(coda_concorrente.coda->dim);
        strncpy(coda_concorrente.coda -> nome , nomeFile , coda_concorrente.coda -> dim);
        coda_concorrente.coda -> next = NULL;
        coda_concorrente.last = coda_concorrente.coda;

    }
    else{

        //ci sono nodi quindi appendo all'ultimo
        NodoCoda * nuovoNodo = NULL;

        nuovoNodo = s_malloc(sizeof(NodoCoda));
        nuovoNodo -> dim = strnlen ( nomeFile , MAX_NAME) + 1;
        nuovoNodo -> nome = s_malloc((nuovoNodo)->dim);
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



/*
 * pop della coda concorrente
 * */
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


    char * fileName = s_malloc(coda_concorrente.coda->dim);

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

        no_more_files = 1;
        BCAST( &coda_cond )
        UNLOCK(&coda_mutex)
        return fileName;

    }

    SIGNAL( &coda_cond )
    UNLOCK( &coda_mutex )

    return fileName;

}


/*
 * inserimento in coda messaggi
 * */
void insert_Coda_Mes(Nodo_Lista_Mes **lista, Nodo_Lista_Mes **last, Nodo_Lista_Mes  * Ins){

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
        BCAST(&mes_list_cond)
        UNLOCK(&mes_list_mutex)
        return;

    }

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

}


/*
 * funzione che fa la pop della coda messaggi
 * */
Mes * pop_Coda_Mes (){

    LOCK(&mes_list_mutex)

    while (!(Coda_Mes_ptr) && !end_list){

        WAIT ( &mes_list_cond , &mes_list_mutex )

    }

    if(!(Coda_Mes_ptr) ){

        SIGNAL(&mes_list_cond)
        UNLOCK(&mes_list_mutex)
        return NULL;

    }

    if(!strncmp(Coda_Mes_ptr -> msg -> nome , "quit" , 4 )){

        free(Coda_Mes_ptr -> msg -> nome);
        free(Coda_Mes_ptr -> msg);
        free(Coda_Mes_ptr);
        Coda_Mes_ptr = NULL;
        end_list = 1;

        SIGNAL(&mes_list_cond)
        UNLOCK(&mes_list_mutex)

        return NULL;

    }
    Nodo_Lista_Mes * next = (Coda_Mes_ptr) -> next;

    size_t len = strnlen(Coda_Mes_ptr -> msg -> nome, MAX_NAME) + 1;
    Mes * ret = s_malloc(sizeof(Mes));
    ret -> nome = s_malloc(len);
    strncpy(ret -> nome , (Coda_Mes_ptr) -> msg -> nome , len  );
    ret -> val = (Coda_Mes_ptr) -> msg -> val;

    free((Coda_Mes_ptr) -> msg -> nome);
    free( (Coda_Mes_ptr) -> msg);
    free(Coda_Mes_ptr);

    (Coda_Mes_ptr) = next;
    if(next == NULL){

        (last_Mes_Ptr) = NULL;

    }

    SIGNAL(&mes_list_cond)
    UNLOCK(&mes_list_mutex)

    return ret;

}

/*
 * funzione invocata dai threads che fa il calcolo del file
 * e inserisce in coda mesaggi
 * */
void worker_Fun(void* filepath){

    //dichiaro le variabili
    char * filePath = (char*) filepath;

    FILE * fd = NULL;
    long int i,lBuf,retValue;

    size_t filePathLen = strnlen(filePath , MAX_NAME) + 1;

    //apro il file
    if((fd = fopen (filePath , "r")) == NULL){

        fprintf(stderr,"ERRORE fopen nel file: %s\n",filePath);
        return;

    }

    retValue = 0;
    i=0;
    //calcolo il valore
    while(fread (&lBuf , sizeof (long int) ,1,fd) != sizeof(long int) && !feof(fd)){

        retValue += lBuf * (i++);

    }
    //se si è interrotto per motivi diversi da EOF
    if(!feof (fd)){

        if(fclose (fd) == EOF){

            int err = errno;
            perror("fclose :");
            exit(err);

        }
        fprintf (stderr,"ERRORE thread worker fread nel file:%s\n",filePath);
        return;

    }

    //chiudo il file

    if((errno = 0),fclose (fd) == EOF){

        int err = errno;
        perror("fclose :");
        exit(err);

    }


    //creo il nuovo nodo
    Nodo_Lista_Mes * nuovo = NULL;
    nuovo = s_malloc(sizeof(NodoCoda));
    nuovo -> msg = s_malloc(sizeof(Mes));
    nuovo -> msg -> nome = s_malloc(filePathLen);
    strncpy (nuovo -> msg -> nome , filePath , filePathLen);
    nuovo -> msg -> val = retValue;
    free(filepath);

    //inserisco il nodo in lista
    insert_Coda_Mes(&Coda_Mes_ptr, &last_Mes_Ptr, nuovo);


}

/*
 * funzione worker che prende dalla coda concorrente e chiama la workerFun
 *
 * */
void * worker(){


    char* nomeFile;

    while(1) {

        if (((nomeFile = pop_Coda_Con ()) && !strncmp ( nomeFile , "quit" , 4)) ) {



            LOCK(&ter_mes_mutex)

            terMes--;

            UNLOCK(&ter_mes_mutex)

            free(nomeFile);

            return NULL;

        }

        if(!nomeFile || signExit){

            if(nomeFile) {

                free(nomeFile);
                nomeFile = NULL;
            }
            LOCK(&ter_mes_mutex)
            if(terMes > 1){

                terMes--;

                UNLOCK(&ter_mes_mutex)

                return NULL;

            }else{

                UNLOCK(&ter_mes_mutex)

                Nodo_Lista_Mes * ultimo = NULL;
                ultimo = s_malloc(sizeof(NodoCoda));
                ultimo -> msg = s_malloc(sizeof(Mes));
                ultimo -> msg -> nome = s_malloc(5 * sizeof(char));
                strncpy (ultimo -> msg -> nome , "quit" , 5);
                ultimo -> msg -> val = MAXLONG;

                insert_Coda_Mes(&Coda_Mes_ptr, &last_Mes_Ptr, ultimo);

                return NULL;

            }



        }

        worker_Fun(nomeFile);

    }


}

/*
 * thread dedicato a scirvere messaggi sulla socket
 * */
void * sender(void * err) {

    int fd_sock ,e;

    IS_MENO1(fd_sock = socket (AF_UNIX , SOCK_STREAM , 0 ) , "errore creazione socket:" , exit(EXIT_FAILURE))

    struct sockaddr_un sa;

    //imposto il tempo di attesa per riprovare la connect 1 secondo
    struct timespec wait;

    wait.tv_nsec = 50000000;
    wait.tv_sec = 0;

    sa.sun_family = AF_UNIX;
    strncpy( sa.sun_path , SOCK_NAME , SOCK_NAME_LEN );
    sa.sun_path[SOCK_NAME_LEN] = '\0';

    //provo per 10 volte a connettere
    int i;
    for(i = 0; i < 10 ; i++){

        if((errno = 0) , connect(fd_sock , (struct sockaddr *)&sa , 108) == 0){

            break;

        }

        if(errno != ENOENT){

            e = errno;
            perror("connect ");
            exit(e);

        } else{

            if((errno = 0),nanosleep(&wait, NULL) == -1){

                e= errno;
                perror("nanosleep del sender :");
                exit(e);

            }

        }

    }
    if(i==10){

        fprintf(stderr,"connect fallita\n");
        exit(EXIT_FAILURE);

    }


    Mes * to_send = NULL;
    size_t w_bites  =0;
    int checkPrint;
    while(1){

        checkPrint = printM;

        if(!checkPrint) {

            to_send = pop_Coda_Mes();
            if (to_send) {

                w_bites = strnlen(to_send->nome, MAX_NAME) + 1;


                IS_MENO1(write_n(fd_sock, &w_bites, sizeof(size_t)),"srittura numero bytes :", exit(EXIT_FAILURE))


                IS_MENO1(write_n(fd_sock, to_send->nome, w_bites),"scrittura messaggio :", exit(EXIT_FAILURE))


                IS_MENO1(write_n(fd_sock, &(to_send->val), sizeof(long int)),"scrittura valore :", exit(EXIT_FAILURE))


                free(to_send->nome);
                free(to_send);

            } else {

                w_bites = -2;

                IS_MENO1(write_n(fd_sock, &w_bites, sizeof(size_t)),"write quit :", exit(EXIT_FAILURE))

                if ((errno = 0), close(fd_sock) == -1) {

                    perror("socket close ");
                    exit(EXIT_FAILURE);

                }

                return NULL;


            }

        }
        else{

            printM = 0;

            w_bites =-3;

            IS_MENO1(write_n(fd_sock, &w_bites, sizeof(size_t)) , "write print request ", exit(EXIT_FAILURE))


        }

    }

}