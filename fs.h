#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "lib/hash.h"

typedef struct tecnicofs {
    node **bstRoot;
    int nextINumber;
    int nBuckets;
} tecnicofs;

int obtainNewInumber(tecnicofs *fs);

tecnicofs *new_tecnicofs(int n_buckets);
void free_tecnicofs(tecnicofs *fs);

void create(tecnicofs *fs, char* name, int inumber, int hash);
void delete(tecnicofs *fs, char* name, int hash);
int lookup(tecnicofs *fs, char* name, int hash);
void change_name(tecnicofs *fs, char *oldName, char *newName, int hash);

void print_tecnicofs_tree(FILE *fp, tecnicofs *fs);

#endif /* FS_H */
