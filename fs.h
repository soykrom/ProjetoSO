#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "lib/hash.h"

typedef struct tecnicofs {
    hash_node* buckets;
    int nBuckets;
    int nextINumber;
} tecnicofs;

int obtainNewInumber(tecnicofs *fs);

tecnicofs *new_tecnicofs(int n_buckets);
void free_tecnicofs(tecnicofs *fs);

void create(tecnicofs *fs, char* name, int inumber);
void delete(tecnicofs *fs, char* name);
int lookup(tecnicofs *fs, char* name);

void print_tecnicofs_tree(FILE *fp, tecnicofs *fs);

#endif /* FS_H */
