#include "locks.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//If the creation of locks is unsuccessful, will exit the program.
void create_locks() {
    MUTEX_INIT(&lockM);
    MUTEX_INIT(&lockFS);

    RWLOCK_INIT(&rwlockM);
    RWLOCK_INIT(&rwlockFS);
}

//If the destruction of locks is unsuccessful, will exit the program.
void destroy_locks() {
    MUTEX_DESTROY(&lockM);
    MUTEX_DESTROY(&lockFS);

    RW_DESTROY(&rwlockM);
    RW_DESTROY(&rwlockFS);
}