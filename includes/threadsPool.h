#ifndef THREADS_POOL
#define THREADS_POOL

#include <pthread.h>
#include <sys/types.h>




typedef struct nodoLista{//Typo lista di messaggi

    Mes * msg;
    struct nodoLista * next;

}Nodo_Lista_Mes;

typedef struct nodoCoda{

    char*nome;
    int dim;
    struct nodoCoda * next;

}NodoCoda;

typedef struct codaCon{

    struct timespec * delay;
    long lim;
    long curr;
    long th_number;
    NodoCoda * last;
    NodoCoda * coda;

}CodaCon;

extern char dCase;
extern int is_set_coda_cond,end_list,no_more_files;
extern CodaCon coda_concorrente;
extern Nodo_Lista_Mes * l_Proc_Ptr;
extern Nodo_Lista_Mes * last_Proc_Ptr;



extern pthread_mutex_t mes_list_mutex;
extern pthread_cond_t mes_list_cond;
extern pthread_mutex_t coda_mutex;
extern pthread_cond_t coda_cond;



void insertCoda(Nodo_Lista_Mes **lista,Nodo_Lista_Mes **last,Nodo_Lista_Mes * ins);

void * worker(void * e);

int worker_Fun(void* filepath);

char * pop_Coda_Con();

int insert_coda_con(char * nomeFile);

void init_coda_con();

void set_standard_coda_con();

void printList (Nodo_Lista_Mes *lptr);

void printListCoda (NodoCoda *lptr);

Mes * popListMes (Nodo_Lista_Mes ** lPtr, Nodo_Lista_Mes ** last);

#endif