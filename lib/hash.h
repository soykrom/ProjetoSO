#ifndef HASH_H
#define HASH_H 1
#include "bst.h"
#include <pthread.h>

typedef struct hash_node{
  node *bstRoot;
  pthread_mutex_t bstlock;
}*hash_node;

int hash(char* name, int n_buckets);
hash_node *create_hashtable(int n_buckets);
void delete_hash(hash_node* buckets, int n_buckets);


#endif
