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

	if(tfsCreate("c", 2, 1) == 0) printf("fixe\n");
	if(tfsCreate("dr", 2, 1) == 0) printf("mega fixe\n");
	if(tfsCreate("a", 2, 1) == 0) printf("se leres isto es gay\n");

	tfsUnmount();

	return 0;


}
