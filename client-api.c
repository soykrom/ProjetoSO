#include "tecnicofs-client-api.h"
#include <stdio.h>

#define MAXLINHA 512

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s sock_path\n", argv[0]);
		exit(0);
	}

	tfsMount(argv[1]);

	if(tfsOpen("hello", 2) == TECNICOFS_ERROR_FILE_NOT_FOUND) printf("good\n");

	tfsCreate("hi", 2, 1);
	tfsCreate("hello", 2, 1);

	if(tfsOpen("hello", 1) == TECNICOFS_ERROR_INVALID_MODE) printf("pretty good\n");
	if(tfsOpen("hello", 2) == 0) printf("also good\n");

	tfsCreate("gay", 2, 1);
	tfsDelete("hello");
	tfsRename("hi", "suh");

	tfsUnmount();

	return 0;


}
