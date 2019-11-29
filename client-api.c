#include "tecnicofs-client-api.h"
#include <stdio.h>
#include "unix.h"

#define MAXLINHA 512

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s sock_path\n", argv[0]);
		exit(0);
	}

	tfsMount(argv[1]);

	tfsCreate("hi", 2, 1);
	tfsCreate("hello", 2, 1);
	tfsCreate("gay", 2, 1);
	tfsDelete("hello");
	tfsRename("hi", "suh");
	
	tfsUnmount();

	return 0;


}
