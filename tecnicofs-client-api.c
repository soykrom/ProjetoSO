#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>


#define MAXLINHA 512

int sockfd = 0;

int tfsMount(char *address){
  int servlen;
  struct sockaddr_un serv_addr;

  if(sockfd != 0){
    exit(TECNICOFS_ERROR_OPEN_SESSION);
  }
  if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, address);
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
    exit(TECNICOFS_ERROR_CONNECTION_ERROR);

  return 0;
}

int tfsUnmount(){
  if(sockfd == 0)
    exit(TECNICOFS_ERROR_NO_OPEN_SESSION);

  if(close(sockfd) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  return 0;
}

int tfsRead(int fd, char *buffer, int len) {
  return read(fd, buffer, len);
}

int tfsWrite(int fd, char *buffer, int len) {
  return write(fd, buffer, len);
}