#ifndef HASH_H
#define HASH_H 
#include "bst.h"
#include <pthread.h>

typedef struct hash_node {
  node *bstRoot;
  pthread_mutex_t bstlock;
} *hash_node;

int hash(char* name, int n_buckets);

#endif
