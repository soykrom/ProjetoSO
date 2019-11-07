#ifndef LOCKS_H
#define LOCKS_H
#include "lib/bst.h"
#include "fs.h"


#ifdef MUTEX
//Makefile replaces the variables with the corresponding code, creating an .exe with mutexs.
pthread_mutex_t mLock;
pthread_mutex_t *locks;

#define LOCK(X) if(pthread_mutex_lock(X)) exit(EXIT_FAILURE);
#define RD_LOCK(X) LOCK(X);
#define UNLOCK(X) if(pthread_mutex_unlock(X)) exit(EXIT_FAILURE);


#elif RWLOCK
//Makefile replaces the variables with the corresponding code, creating an .exe with read_write_locks.#define MUTEX_LOCK(X)
pthread_rwlock_t mLock;
pthread_rwlock_t *locks;

#define LOCK(X) if(pthread_rwlock_wrlock(X)) exit(EXIT_FAILURE);
#define RD_LOCK(X) if(pthread_rwlock_rdlock(X)) exit(EXIT_FAILURE);
#define UNLOCK(X) if(pthread_rwlock_unlock(X)) exit(EXIT_FAILURE);

#else
void *mLock;
void *locks;

#define LOCK(X)
#define RD_LOCK(X)
#define UNLOCK(X)

#endif

void create_locks(tecnicofs *fs);

void destroy_locks(tecnicofs *fs);

#endif