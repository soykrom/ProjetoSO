#ifndef LOCKS_H
#define LOCKS_H
#include "lib/bst.h"
#include "fs.h"


/* ALTERAR MACROS PARA NAO TER QUE USAR AS DUAS. RECEBE A POSICAO DA HASH E DA LOCK DEPENDENDO SE E MUTEX OU RWLOCK 

Problemas: 1 - Distinguir entre rdlock e rwlock
            Solução: Apenas no comamdo l usar o rdlock diretamente, e não chamar a função*/

#ifdef MUTEX
    
//Makefile replaces the variables with the corresponding code, creating an .exe with mutexs.
#define MUTEX_INIT(X) if(pthread_mutex_init(X, NULL)) exit(EXIT_FAILURE);
#define MUTEX_LOCK(X) if(pthread_mutex_lock(X)) exit(EXIT_FAILURE);
#define MUTEX_UNLOCK(X) if(pthread_mutex_unlock(X)) exit(EXIT_FAILURE);
#define MUTEX_DESTROY(X) if(pthread_mutex_destroy(X)) exit(EXIT_FAILURE);
#define RWLOCK_INIT(X)
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X)
#define RW_DESTROY(X)

#elif RWLOCK

//Makefile replaces the variables with the corresponding code, creating an .exe with read_write_locks.
#define MUTEX_INIT(X)
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define MUTEX_DESTROY(X)
#define RWLOCK_INIT(X) if(pthread_rwlock_init(X, NULL)) exit(EXIT_FAILURE);
#define RW_LOCK(X) if(pthread_rwlock_wrlock(X)) exit(EXIT_FAILURE);
#define RD_LOCK(X) if(pthread_rwlock_rdlock(X)) exit(EXIT_FAILURE);
#define RW_UNLOCK(X) if(pthread_rwlock_unlock(X)) exit(EXIT_FAILURE);
#define RW_DESTROY(X) if(pthread_rwlock_destroy(X)) exit(EXIT_FAILURE);

#else

//Makefile replaces the variables with the corresponding code, creating an .exe without sync.
#define MUTEX_INIT(X)
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define MUTEX_DESTROY(X)
#define RWLOCK_INIT(X)
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X)
#define RW_DESTROY(X)
#endif


void create_locks(tecnicofs *fs);


void destroy_locks(tecnicofs *fs);

#endif