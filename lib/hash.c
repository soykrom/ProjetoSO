#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
/* Simple hash function for strings.
 * Receives a string and resturns its hash value
 * which is a number between 0 and n-1
 * In case the string is null, returns -1 */
int hash(char* name, int n_buckets) {
	if (!name)
		return -1;
	return (int) name[0] % n_buckets;
}

hash_node *create_hashtable(int n_buckets){
	int i;
	hash_node *buckets;

	buckets =(hash_node*) malloc(n_buckets * sizeof(hash_node));
	for(i = 0; i < n_buckets; i++) buckets[i] = NULL;

	return buckets;
}

void delete_hash(hash_node* buckets, int n_buckets){
	int i;

	for(i = 0; i < n_buckets; i++){
		free(buckets[i]);
	}
	free(buckets);
}
