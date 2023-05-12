#ifndef THREADS_POOL
#define THREADS_POOL
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <sys/types.h>
#include <values.h>

#include <util.h>

extern volatile __sig_atomic_t signExit, printM;


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

extern int terMes,is_set_coda_con,end_list,no_more_files;


//coda concorrente e lista di messaggi
extern CodaCon coda_concorrente;
extern Nodo_Lista_Mes * Coda_Mes_ptr;
extern Nodo_Lista_Mes * last_Mes_Ptr;

//mutex
extern pthread_mutex_t ter_mes_mutex;
extern pthread_mutex_t mes_list_mutex;
extern pthread_mutex_t coda_mutex;

//cond per le varie wait
extern pthread_cond_t mes_list_cond;
extern pthread_cond_t coda_cond;

void insert_Coda_Mes(Nodo_Lista_Mes **lista, Nodo_Lista_Mes **last, Nodo_Lista_Mes * Ins);

void * worker();

void worker_Fun(void* filepath);

int insert_coda_con(char * nomeFile);

Mes * pop_Coda_Mes ();

void * sender(void *);

#endif