#include "tecnicofs-client-api.h"
#include <stdio.h>
#include "unix.h"

#define MAXLINHA 512

int main(int argc, char *argv[]) {
	int socket;

	if (argc != 2) {
		printf("Usage: %s sock_path\n", argv[0]);
		exit(0);
	}

	socket = tfsMount(argv[1]);
	char buffer[MAXLINHA] = "Ola";

	write(socket, buffer, strlen(buffer) + 1);

	read(socket, buffer, MAXLINHA + 1);

	printf("Client - %s\n", buffer);
	tfsUnmount();

	return 0;


}
	
