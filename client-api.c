#include "tecnicofs-client-api.h"
#include <stdio.h>
#include "unix.h"

int main() {
	int i, j;
	i = tfsMount(UNIXSTR_PATH);

	j = tfsUnmount(UNIXSTR_PATH);

	printf("\ni - %d    ;    j - %d", i, j);
	return 0;


}
	
