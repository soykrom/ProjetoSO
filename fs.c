#include "fs.h"
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



/*int obtainNewInumber(tecnicofs *fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}*/

tecnicofs* new_tecnicofs(int n_buckets) {
	tecnicofs *fs = (tecnicofs*) malloc(sizeof(tecnicofs));

	if(!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}

	fs->nBuckets = n_buckets;
	fs->bstRoot = (node**) malloc(sizeof(node*) * fs->nBuckets);
	if(!fs->bstRoot) {
		perror("failed to allocate fs->bstRoot");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < fs->nBuckets; i++) fs->bstRoot[i] = NULL;

  inode_table_init();
	return fs;
}

void free_tecnicofs(tecnicofs *fs) {
	int i;

	destroy_locks(fs);
	for(i = 0; i < fs->nBuckets; i++) {
		free_tree(fs->bstRoot[i]);
	}

  inode_table_destroy();
	free(fs->bstRoot);
	free(fs);
}



void create(tecnicofs *fs, char *name, int inumber, int i) {

	fs->bstRoot[i] = insert(fs->bstRoot[i], name, inumber);
}

int lookup(tecnicofs *fs, char *name, int i) {
	node *searchNode = search(fs->bstRoot[i], name);

	if (searchNode) return searchNode->inumber;

	return 0;
}

void delete(tecnicofs *fs, char *name, int i) {
	fs->bstRoot[i] = remove_item(fs->bstRoot[i], name);
}

void change_name(tecnicofs *fs, char *oldName, char *newName, int h1) {
	int h2 = hash(newName, fs->nBuckets);

	if(h1 == h2) {
		LOCK(&locks[h1]);

		int inumber = lookup(fs, oldName, h1);

		if(inumber && !lookup(fs, newName, h1)) {
			fs->bstRoot[h1] = remove_item(fs->bstRoot[h1], oldName);
			fs->bstRoot[h1] = insert(fs->bstRoot[h1], newName, inumber);
		}

		UNLOCK(&locks[h1]);
	} else { //h1 != h2
		if(h1 > h2) { //Dar Lock por ordem decrescente hash de maneira a evitar situacoes de deadlock
			LOCK(&locks[h1]);
			LOCK(&locks[h2]);
		} else {
			LOCK(&locks[h2]);
			LOCK(&locks[h1]);
		}

		int inumber = lookup(fs, oldName, h1);

		if(inumber && !lookup(fs, newName, h2)) {
			fs->bstRoot[h1] = remove_item(fs->bstRoot[h1], oldName);
			fs->bstRoot[h2] = insert(fs->bstRoot[h2], newName, inumber);
		}

		UNLOCK(&locks[h1]);
		UNLOCK(&locks[h2]);
	}
}

void print_tecnicofs_tree(FILE *fp, tecnicofs *fs) {
	int i;
	for(i = 0; i < fs -> nBuckets; i++) {
		if(!fs->bstRoot[i]) continue;

		print_tree(fp, fs->bstRoot[i]);
	}

	fclose(fp);
}
