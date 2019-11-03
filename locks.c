#include "locks.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//If the creation of locks is unsuccessful, will exit the program.
void create_locks(tecnicofs *fs) {
    int i = 0;

    MUTEX_INIT(&mLock);

    fs->locksM = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * fs->nBuckets); 
    if(!fs->locksM) {
        perror("failed to allocate locks");
        exit(EXIT_FAILURE);
    }

    for(; i < fs->nBuckets; i++) {
        MUTEX_INIT(&fs->locksM[i]);
    }

    i = 0;

    RWLOCK_INIT(&rwLock);

    fs->locksRW = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t) * fs->nBuckets); 
    if(!fs->locksRW) {
        perror("failed to allocate locks");
        exit(EXIT_FAILURE);
    }

    for(; i < fs->nBuckets; i++) {
        RWLOCK_INIT(&fs->locksRW[i]);
    }

}

//If the destruction of locks is unsuccessful, will exit the program.
void destroy_locks(tecnicofs *fs) {
    int i = 0;

    MUTEX_DESTROY(&mLock);

    for(; i < fs->nBuckets; i++) {
        MUTEX_DESTROY(&fs->locksM[i]);
    }

    free(fs->locksM);

    i = 0;

    RW_DESTROY(&rwLock);

    for(; i < fs->nBuckets; i++) {
        RW_DESTROY(&fs->locksRW[i]);
    }
    
    free(fs->locksRW);

}