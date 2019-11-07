#include "locks.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//If the creation of locks is unsuccessful, will exit the program.
void create_locks(tecnicofs *fs) {
    #ifdef MUTEX
    int i = 0;

    if(pthread_mutex_init(&mLock, NULL)) exit(EXIT_FAILURE);

    locks = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * fs->nBuckets); 
    if(!locks) {
        perror("failed to allocate locks");
        exit(EXIT_FAILURE);
    }

    for(; i < fs->nBuckets; i++) {
        if(pthread_mutex_init(&locks[i], NULL)) exit(EXIT_FAILURE);
    }


    #elif RWLOCK
    int i = 0;

    if(pthread_rwlock_init(&mLock, NULL)) exit(EXIT_FAILURE);

    locks = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t) * fs->nBuckets); 
    if(!locks) {
        perror("failed to allocate locks");
        exit(EXIT_FAILURE);
    }

    for(; i < fs->nBuckets; i++) {
        if(pthread_rwlock_init(&locks[i], NULL)) exit(EXIT_FAILURE);
    }

    #endif
}


//If the destruction of locks is unsuccessful, will exit the program.
void destroy_locks(tecnicofs *fs) {
    #ifdef MUTEX
    int i = 0;

    if(pthread_mutex_destroy(&mLock)) exit(EXIT_FAILURE);

    for(; i < fs->nBuckets; i++) {
        if(pthread_mutex_destroy(&locks[i])) exit(EXIT_FAILURE);
    }

    free(locks);

    #elif RWLOCK
    int i = 0;

    if(pthread_rwlock_destroy(&mLock)) exit(EXIT_FAILURE);

    for(; i < fs->nBuckets; i++) {
        if(pthread_rwlock_destroy(&locks[i])) exit(EXIT_FAILURE);
    }
    
    free(locks);

    #endif
}