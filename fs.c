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
	tecnicofs *fs = malloc(sizeof(tecnicofs));

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

/*
void change_name(tecnicofs *fs, char *oldName, char *newName) {
	int iNumber = lookup(fs, oldName);

	if(iNumber || !lookup(fs, newName)) {
		delete(fs, oldName);
		create(fs, newName, iNumber);
	}
}
*/

void print_tecnicofs_tree(FILE *fp, tecnicofs *fs) {
	int i;
	for(i = 0; i < fs -> nBuckets; i++) {
		if(!fs->bstRoot[i]) continue;

		print_tree(fp, fs->bstRoot[i]);
	}

	fclose(fp);
}
