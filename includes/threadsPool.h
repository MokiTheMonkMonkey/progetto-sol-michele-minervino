#ifndef THREADS_POOL
#define THREADS_POOL

#include <pthread.h>
#include <sys/types.h>
#include <values.h>

#include <util.h>
#include "util.h"

extern volatile sig_atomic_t signExit, printM;

typedef struct nodoLista{//Typo lista di messaggi

    Mes * msg;
    struct nodoLista * next;

}Nodo_Lista_Mes;

typedef struct nodoCoda{//nodo della coda concorrente

    char *nome;
    size_t dim;
    struct nodoCoda * next;

}NodoCoda;

typedef struct codaCon{//coda concorrente

    struct timespec * delay;
    long lim;
    long curr;
    long th_number;
    pthread_t *workers;
    NodoCoda * last;
    NodoCoda * coda;

}CodaCon;

//variabili condivise

extern int is_set_coda_cond,end_list,no_more_files;
extern long terMes;

//coda concorrente e lista di messaggi
extern CodaCon coda_concorrente;
extern Nodo_Lista_Mes * l_Proc_Ptr;
extern Nodo_Lista_Mes * last_Proc_Ptr;

//mutex
extern pthread_mutex_t ter_mes_mutex;
extern pthread_mutex_t mes_list_mutex;
extern pthread_mutex_t coda_mutex;

//cond per le varie wait
extern pthread_cond_t mes_list_cond;
extern pthread_cond_t coda_cond;

void insertCoda(Nodo_Lista_Mes **lista,Nodo_Lista_Mes **last,Nodo_Lista_Mes * ins);

void * worker();

int worker_Fun(void* filepath);

int insert_coda_con(char * nomeFile);

Mes * popListMes ();

void * sender(void *);

#endif