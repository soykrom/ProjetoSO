#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>



#define UNIXSTR_PATH "/tmp/socket.unix.stream"
#define UNIXDG_PATH "/tmp/socket.unix.datagram"

#define UNIXDG_TMP "/tmp/socket.unix.dgXXXXXXX"

#define MAXLINHA 512
