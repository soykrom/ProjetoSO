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

	if(tfsCreate("hi", 2, 1) == 0) printf("fixe\n");
	//Com 1 create da bem.
	//if(tfsCreate("hello", 2, 1) == 0) printf("mega fixe\n");

	tfsUnmount();

	return 0;


}
