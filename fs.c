#include "fs.h"
#include "locks.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int obtainNewInumber(tecnicofs *fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

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

	fs->nextINumber = 0;
	return fs;
}

void free_tecnicofs(tecnicofs *fs) {
	int i;

	destroy_locks(fs);
	for(i = 0; i < fs->nBuckets; i++) {
		free_tree(fs->bstRoot[i]);
	}
	
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

		node *old = search(fs->bstRoot[h1], oldName);
		if(!old) {
			UNLOCK(&locks[h1]);

			return;
		}

		if(!search(fs->bstRoot[h1], newName)) {
			fs->bstRoot[h1] = insert(fs->bstRoot[h1], newName, old->inumber);
			fs->bstRoot[h1] = remove_item(fs->bstRoot[h1], oldName);
		}

		UNLOCK(&locks[h1]);
	} else { //h1 != h2
		if(h1 > h2) { //Dar Lock por ordem de maior hash de maneira a evitar situações de deadlock
			LOCK(&locks[h1]); 
			LOCK(&locks[h2]);
		 } else {
			LOCK(&locks[h2]); 
			LOCK(&locks[h1]);
		 }

		node *old = search(fs->bstRoot[h1], oldName);

		if(!old) {
			UNLOCK(&locks[h1]);

			return;
		}

		if(!search(fs->bstRoot[h2], newName)) {
			fs->bstRoot[h2] = insert(fs->bstRoot[h2], newName, old->inumber);
			fs->bstRoot[h1] = remove_item(fs->bstRoot[h1], oldName);
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
