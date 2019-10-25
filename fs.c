#include "fs.h"
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
	hash_node *buckets = create_hashtable(n_buckets);
	fs->buckets = buckets;
	fs->nextINumber = 0;
	fs->nBuckets = n_buckets;
	return fs;
}

void free_tecnicofs(tecnicofs *fs) {
	int i;
	for(i = 0; i < fs -> nBuckets; i++){
		free_tree(fs -> buckets[i] -> bstRoot);
	}
	delete_hash(fs -> buckets, fs -> nBuckets);
	free(fs);
}

void create(tecnicofs *fs, char* name, int inumber) {
	int i = hash(name, fs->nBuckets);
	fs->buckets[i]->bstRoot = insert(fs->buckets[i]->bstRoot, name, inumber);
}

void delete(tecnicofs *fs, char* name) {
	int i = hash(name, fs->nBuckets);
	fs->buckets[i]->bstRoot = remove_item(fs->buckets[i]->bstRoot, name);
}

int lookup(tecnicofs *fs, char* name) {
	int i = hash(name, fs->nBuckets);
	node *searchNode = search(fs->buckets[i]->bstRoot, name);

	if (searchNode) return searchNode->inumber;

	return 0;
}

void print_tecnicofs_tree(FILE *fp, tecnicofs *fs) {
	int i;
	for(i = 0; i < fs -> nBuckets; i++) print_tree(fp, fs->buckets[i]->bstRoot);
	fclose(fp);
}
