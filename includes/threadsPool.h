#ifndef THREADS_POOL
#define THREADS_POOL

#include <pthread.h>
#include <sys/types.h>

/* controlla se s non e' 0,stampa errore e termina*/
#define NOT_ZERO(s,m){ \
        if( ( s )  != 0 ){ \
            perror ( m ) ;  exit ( EXIT_FAILURE ) ;\
        }\
}
/* controlla < 0; stampa errore e termina */
#define ec_min_zero(s,m) \
 if ( (s) < 0 ) {perror(m); exit(EXIT_FAILURE);}
/*controlla <= 0;stampa errore e termina */
#define ec_mu_zero(s,m) \
 if ( (s) <= 0 ) {perror(m); exit(EXIT_FAILURE);}
/* controlla NULL; stampa errore e termina (NULL) */
#define ec_null(s,m) \
 if((s)==NULL) {perror(m); exit(EXIT_FAILURE);}
/* controlla -1; stampa errore ed esegue c */
#define ec_meno1_c(s,m,c) \
 if((s)==-1) {perror(m); c;}
#define LOCK(l)      if (pthread_mutex_lock(l)!=0)        { \
    fprintf(stderr, "ERRORE FATALE lock\n");		    \
    pthread_exit((void*)EXIT_FAILURE);			    \
  }
#define UNLOCK(l)    if (pthread_mutex_unlock(l)!=0)      { \
  fprintf(stderr, "ERRORE FATALE unlock\n");		    \
  pthread_exit((void*)EXIT_FAILURE);				    \
  }
#define WAIT(c,l)    if (pthread_cond_wait(c,l)!=0)       { \
    fprintf(stderr, "ERRORE FATALE wait\n");		    \
    pthread_exit((void*)EXIT_FAILURE);				    \
}
#define SIGNAL(c)    if (pthread_cond_signal(c)!=0)       {	\
    fprintf(stderr, "ERRORE FATALE signal\n");			\
    pthread_exit((void*)EXIT_FAILURE);					\
  }
#define BCAST(c)     if (pthread_cond_broadcast(c)!=0)    {		\
    fprintf(stderr, "ERRORE FATALE broadcast\n");			\
    pthread_exit((void*)EXIT_FAILURE);						\
  }
#define ISSET_CODA( s , m ) if ( s != -1 )      { \
    fprintf(stderr, m);                           \
    return -1;      \
}



typedef struct nodoLista{//Typo lista di messaggi

    char * nome;
    long int val;

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

extern int is_set_coda_cond,cond_Master;
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


#endif