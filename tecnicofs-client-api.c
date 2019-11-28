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


int tfsMount(char *address) {
  int servlen;
  struct sockaddr_un serv_addr;

  if(sockfd != 0) {
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
  int n = 0;

  if(sockfd == 0)
    exit(TECNICOFS_ERROR_NO_OPEN_SESSION);

  char buffer[MAXLINHA];

  strcpy(buffer, "e");

  n = strlen(buffer) + 1;
  if(write(sockfd, buffer, n) != n) 
    exit(TECNICOFS_ERROR_OTHER);

  if(read(sockfd, buffer, MAXLINHA + 1) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  if(atoi(buffer) != 1 && close(sockfd) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  return 0;
}

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
  char buffer[MAXLINHA];
  int n;

  sprintf(buffer, "c %s %d%d", filename, ownerPermissions, othersPermissions);

  n = strlen(buffer) + 1;

  if(write(sockfd, buffer, n) != n)
    exit(TECNICOFS_ERROR_OTHER);

  if(read(sockfd, buffer, MAXLINHA + 1) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  if(atoi(buffer) == 1) 
    return 0;
  
  return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
}

int tfsDelete(char *filename) {
  char buffer[MAXLINHA];
  int n;

  sprintf(buffer, "d %s", filename);

  n = strlen(buffer) + 1;
  if(write(sockfd, buffer, n) != n)
    exit(TECNICOFS_ERROR_OTHER);

  if(read(sockfd, buffer, MAXLINHA + 1) < 0)
    exit(TECNICOFS_ERROR_OTHER);

  if(atoi(buffer) == 1)
    return 0;

  return TECNICOFS_ERROR_PERMISSION_DENIED;    
}