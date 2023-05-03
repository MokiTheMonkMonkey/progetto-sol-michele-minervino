#if  !defined(UTIL_H)
#define UTIL_H

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define SOCK_NAME "farm.sck"
#define SOCK_NAME_LEN 9
#define MAX_NAME 256

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
 if ( s <= 0 ) {perror(m); exit(EXIT_FAILURE);}
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
#define ISSET_CODA(s,m) \
    if ( s != 0 )      { \
        fprintf(stderr, m);                           \
        return -1;      \
    }

typedef struct mes {

    char * nome;
    long val;

}Mes;



char * valid_name(char * dirname , char * next);

void* s_malloc (unsigned long size);

long isNumber(const char* s);

size_t readn(long fd, void *buf, size_t size);

size_t writen(int fd, void *buf, size_t size);

#endif
