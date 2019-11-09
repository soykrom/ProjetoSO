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

void create(tecnicofs *fs, char *name, int inumber) {
	int i = hash(name, fs->nBuckets);

	if(!fs->bstRoot[i]) fs->bstRoot[i] = new_node(name, inumber);

	fs->bstRoot[i] = insert(fs->bstRoot[i], name, inumber);
}

int lookup(tecnicofs *fs, char *name) {
	int i = hash(name, fs->nBuckets);
	node *searchNode = search(fs->bstRoot[i], name);

	if (searchNode) return searchNode->inumber;

	return 0;
}

void delete(tecnicofs *fs, char *name) {
	int i = hash(name, fs->nBuckets);

	fs->bstRoot[i] = remove_item(fs->bstRoot[i], name);
}

void change_name(tecnicofs *fs, char *oldName, int pos, char *newName) {
	int h1 = pos;
	int h2 = hash(newName, fs->nBuckets);

	LOCK(&locks[h1]);

	node *old = search(fs->bstRoot[h1], oldName);

	if(!old) {
		UNLOCK(&locks[h1]);

		return;
	}

	if(h1 == h2) {
		if(!search(fs->bstRoot[h1], newName)) {
			create(fs, newName, old->inumber);
			delete(fs, oldName);

			UNLOCK(&locks[h1]);
		} else {
			UNLOCK(&locks[h1]);
			return;
		}

	} else { //h1 != h2
		LOCK(&locks[h2]);

		if(!search(fs->bstRoot[h2], newName)) { //SIGSEV from time to time. Don't know why :/
			create(fs, newName, old->inumber);
			delete(fs, oldName);

			UNLOCK(&locks[h1]);
			UNLOCK(&locks[h2]);
		} else {
			UNLOCK(&locks[h1]);
			UNLOCK(&locks[h2]);

			return;
		}
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
