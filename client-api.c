#include "tecnicofs-client-api.h"
#include <stdio.h>

#define MAXLINHA 512

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s sock_path\n", argv[0]);
		exit(0);
	}
	char buffer[10];

	tfsMount(argv[1]);

	if(tfsOpen("hello", READ) == TECNICOFS_ERROR_FILE_NOT_FOUND) printf("good\n");

	tfsCreate("hi", READ, WRITE);
	tfsCreate("hello", RW, WRITE);

	if(tfsOpen("hello", NONE) == TECNICOFS_ERROR_INVALID_MODE) printf("pretty good\n");
	if(tfsOpen("hello", READ) == 0) printf("also good\n");

	if(tfsOpen("hi", READ) == 1) printf("my peepee is hard\n");

	if(tfsWrite(0, "Zekip", 4) == 0) printf("pls work\n");

	tfsCreate("gay", READ, 1);
	if(tfsRead(0, buffer, 10) == 4) printf("%s\n", buffer);
	//printf("%s\n", buffer);
	if(tfsClose(0) == 0) printf("close works\n");
	tfsDelete("hello");
	tfsRename("hi", "suh");

	tfsUnmount();

	return 0;


}
