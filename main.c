/*Afonso Carvalho    ist193681
  João Gundersen     ist192494
  Grupo 15
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "fs.h"
#include "lib/inodes.h"
#include "macros.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define MAX_CONNECTS 10
#define MAXLINHA 512
#define MAX_OPEN_FILES 5

//Will contain the number of threads.
int numberThreads;
int currentThread = 0;

//Init of the pointer that'll point to the threads.
pthread_t *threads = NULL;

//socket servidor
int sockfd;
struct sockaddr_un end_serv;
int closed;

//locks openFiles
pthread_mutex_t fLock;

typedef struct openFiles{
    int iNumber;
    permission currentMode;
} openFiles;

FILE *fpO;
tecnicofs *fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

void signalHandler() {
    if(close(sockfd) != 0)
        exit(EXIT_FAILURE);

    closed = 1;
}

static void displayUsage (const char* appName) {
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

void errorParse(FILE *fp) {
    fprintf(fp, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void openFilesInit(openFiles UserOpenFiles[]) {
  for(int i = 0; i < MAX_OPEN_FILES; i++) {
    UserOpenFiles[i].iNumber = -2;
    UserOpenFiles[i].currentMode = -2;
  }

  if(pthread_mutex_init(&fLock, NULL)) exit(EXIT_FAILURE);
}

void* clientHandler(void *arg) {
    int *args = (int *) arg;
    char buffer[MAXLINHA + 1];
    int socket = args[0];
    uid_t cli_uid = args[1];
    int iNumber, n, fd, fileFound = 0;
    int currentOpenFiles = 0;
    int msg;
    char token;
    int pos = 0;
    char name[MAX_INPUT_SIZE], otherInfo[MAX_INPUT_SIZE];
    openFiles UserOpenFiles[MAX_OPEN_FILES];
    uid_t uid;
    permission ownerPerm, othersPerm;

    openFilesInit(UserOpenFiles);


    while(1) {
        fileFound = 0;
        if(read(socket, buffer, MAXLINHA + 1) < 0)
            exit(EXIT_FAILURE);

        sscanf(buffer, "%c %s %s", &token, name, otherInfo);
        switch(token) {
            case 'c':
                LOCK(&locks[pos]);

                if(lookup(fs, name, pos) != -1) {
                    msg = -4;

                } else {
                    iNumber = inode_create(cli_uid, atoi(otherInfo)/10, atoi(otherInfo)%10);

                    create(fs, name, iNumber, pos);

                    msg = 0;
                }

                n = sizeof(int);
                if(write(socket, &msg, n) != n)
                    exit(EXIT_FAILURE);

                UNLOCK(&locks[pos]);

                break;
            case 'r':
                LOCK(&locks[pos]);

                if(lookup(fs, name, pos) == -1) {
                    msg = -5;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                    break;
                } else if(lookup(fs, otherInfo, pos) != -1) { //file with new name already exists
                    msg = -4;

                    n = sizeof(int);
                    if(write(socket, buffer, n) != n)
                        exit(EXIT_FAILURE);
                    break;
                }

                inode_get(iNumber, &uid, &ownerPerm, &othersPerm, NULL, 0);

                if(uid != cli_uid) {
                    msg = -6;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);

                    break;
                }

                change_name(fs, name, otherInfo, pos);

                msg = 0;

                n = sizeof(int);
                if(write(socket, &msg, n) != n)
                    exit(EXIT_FAILURE);

                UNLOCK(&locks[pos]);
                break;

            case 'd':
                LOCK(&locks[pos]);

                iNumber = lookup(fs, name, pos);
                if(iNumber == -1) {
                    msg = -5;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                    break;
                }

                pthread_mutex_lock(&fLock);

                for(int i = 0; i < MAX_OPEN_FILES; i++) {
                    if(UserOpenFiles[i].iNumber == iNumber) {
                        fileFound = 1;
                        break;
                    }
                }

                pthread_mutex_unlock(&fLock);

                if(fileFound) {
                    msg = -9;
                } else {
                    delete(fs, name, pos);
                    inode_delete(iNumber);
                    msg = 0;
                }

                n = sizeof(int);
                if(write(socket, &msg, n) != n)
                    exit(EXIT_FAILURE);

                UNLOCK(&locks[pos]);
                break;
            case 'o':
                if(currentOpenFiles == MAX_OPEN_FILES) {
                    msg = -7;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                    break;
                }

                iNumber = lookup(fs, name, pos);


                if(iNumber == -1) {
                    msg = -5;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                    break;
                }

                pthread_mutex_lock(&fLock);

                for(int i = 0; i < MAX_OPEN_FILES; i++) {
                    if(UserOpenFiles[i].iNumber == iNumber) {
                        msg = -9;

                        fileFound = 1;

                        n = sizeof(int);
                        if(write(socket, &msg, n) != n)
                            exit(EXIT_FAILURE);
                        break;
                    }
                }

                if(fileFound) {
                    pthread_mutex_unlock(&fLock);
                    break;
                }

                int perm = atoi(otherInfo);

                inode_get(iNumber, &uid, &ownerPerm, &othersPerm, NULL, 0);

                if((uid == cli_uid && ((ownerPerm == RW && (perm == READ || perm == WRITE)) || perm == ownerPerm)) ||
                (uid != cli_uid && ((othersPerm == RW && (perm == READ || perm == WRITE)) || perm == othersPerm))) {
                    for(int i = 0; i < MAX_OPEN_FILES; i++) {
                        if(UserOpenFiles[i].iNumber == -2) {
                            UserOpenFiles[i].iNumber = iNumber;
                            UserOpenFiles[i].currentMode = perm;

                            msg = i;

                            n = sizeof(int);
                            if(write(socket, &msg, n) != n)
                                exit(EXIT_FAILURE);

                            ++currentOpenFiles;

                            break;
                        }
                    }
                } else {
                    msg = -10;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                }
                pthread_mutex_unlock(&fLock);

                break;
            case 'w':
                fd = atoi(name);

                pthread_mutex_lock(&fLock);

                if(UserOpenFiles[fd].iNumber == -2) {
                    msg = -8;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);

                    pthread_mutex_unlock(&fLock);
                    break;
                }

                if(UserOpenFiles[fd].currentMode == RW || UserOpenFiles[fd].currentMode == WRITE) {
                    if(inode_set(UserOpenFiles[fd].iNumber, otherInfo, strlen(otherInfo)) != 0)
                        exit(EXIT_FAILURE);
                    msg = 0;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                } else {
                    msg = -10;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);

                }
                pthread_mutex_unlock(&fLock);
                break;

            case 'l':
                fd = atoi(name);

                if(pthread_mutex_lock(&fLock)) exit(EXIT_FAILURE);

                if(UserOpenFiles[fd].iNumber == -2) {
                    n = -8;
                    //if(write(socket, &msg, n) != n){
                      //  exit(EXIT_FAILURE);
                    //}
                    //if(pthread_mutex_unlock(&fLock)) exit(EXIT_FAILURE);

                    //break;
                }


                else if(UserOpenFiles[fd].currentMode == RW || UserOpenFiles[fd].currentMode == READ) {
                    n = inode_get(UserOpenFiles[fd].iNumber, NULL, NULL, NULL, buffer, atoi(otherInfo) - 1);
                    if(n == 0) {
                        strcpy(buffer, "");

                        n = strlen(buffer) + 1;
                        //if(write(socket, buffer, n) != n)
                          //  exit(EXIT_FAILURE);
                    } else if(n != strlen(buffer)) {
                        exit(EXIT_FAILURE);
                    } else {
                        n = strlen(buffer) + 1;
                        printf("%s\n", buffer);
                        printf("%d\n", n);
                        //if(write(socket, buffer, n) != n)
                          //  exit(EXIT_FAILURE);
                    }
                } else {

                    n = -10;
                  //  if(write(socket, &msg, n) != n)
                    //    exit(EXIT_FAILURE);
                }
                pthread_mutex_unlock(&fLock);

                if(write(socket, &n, sizeof(int)) != sizeof(int))
                  exit(EXIT_FAILURE);
                if(n > 0){
                  if(write(socket, buffer, n) != n)
                    exit(EXIT_FAILURE);
                }

                break;

            case 'x':
                fd = atoi(name);

                if(UserOpenFiles[fd].iNumber == -2) {
                    msg = -5;

                    n = sizeof(int);
                    if(write(socket, &msg, n) != n)
                        exit(EXIT_FAILURE);
                  break;
                }

                UserOpenFiles[fd].iNumber = -2;
                UserOpenFiles[fd].currentMode = -2;

                --currentOpenFiles;

                msg = 0;

                n = sizeof(int);
                if(write(socket, &msg, n) != n)
                    exit(EXIT_FAILURE);

                break;
            case 'e':
                msg = 0;

                n = sizeof(int);
                if(write(socket, &msg, n) != n)
                    exit(EXIT_FAILURE);

                return NULL;
        }

    }

    return NULL;
}

FILE* openFile(const char *ficheiro, const char *modo) {
    FILE *fp = fopen(ficheiro, modo);

    if(fp == NULL) {
        if(!strcmp(modo, "r"))
            fprintf(stderr, "Error: no input file\n");
        else
            fprintf(stderr, "Error: no output file\n");
        exit(EXIT_FAILURE);
    }

    return fp;
}


int main(int argc, char *argv[]) {
    //Creation of the time variables.
    struct timeval start, end;
    struct ucred client_cred;
    double time;
    int nBuckets;
    int dim_serv;
    socklen_t len_cred;
    char uid[30];

    parseArgs(argc, argv);

    if(signal(SIGINT, signalHandler) == SIG_ERR) {
        fprintf(stderr, "Error invalid signal handler");
        exit(EXIT_FAILURE);
    }

    fpO = openFile(argv[2], "w");

    nBuckets = atoi(argv[3]);
    if(nBuckets != 1) {
        perror("Error: invalid number of buckets");
        exit(EXIT_FAILURE);
    }

    fs = new_tecnicofs(nBuckets);

    inode_table_init();

    create_locks(fs);

    numberThreads = MAX_CONNECTS;
    threads = (pthread_t*) malloc(sizeof(pthread_t*) * numberThreads);
    if(!threads) {
        perror("failed to allocate threads");
        exit(EXIT_FAILURE);
    }

    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket stream servidor");
        exit(EXIT_FAILURE);
    }

    unlink(argv[1]);

    bzero((char *) &end_serv, sizeof(end_serv));

    end_serv.sun_family = AF_UNIX;
    strcpy(end_serv.sun_path, argv[1]);
    dim_serv = strlen(end_serv.sun_path) + sizeof(end_serv.sun_family);

    if(bind(sockfd, (struct sockaddr *) &end_serv, dim_serv)) {
        perror("Erro ao atribuir nome ao socket servidor");
        exit(EXIT_FAILURE);
    }

    //Will save the current time in 'start'.
    gettimeofday(&start, NULL);

    if(listen(sockfd, MAX_CONNECTS) != 0)
        exit(EXIT_FAILURE);

    while(1) {
        int client_socket;
        int args[2];

        numberThreads = currentThread;

        client_socket = accept(sockfd, NULL, NULL);
        if(client_socket < 0) {
            if(closed) break;

            perror("Erro ao criar ligacao dedicada - accept");
            exit(EXIT_FAILURE);
        }

        len_cred = sizeof(struct ucred);
        if(getsockopt(client_socket, SOL_SOCKET, SO_PEERCRED, &client_cred, &len_cred) == -1){
          perror("Erro ao obter as credenciais do cliente");
          exit(EXIT_FAILURE);
        }

        sprintf(uid, "%d", client_cred.uid);
        args[0] = client_socket;
        args[1] = atoi(uid);

        if(pthread_create(&threads[currentThread++], NULL, *clientHandler, (void*)args)) {
            perror("Erro ao criar thread para atender cliente");
            exit(EXIT_FAILURE);
        }
    }

    for(int  i = 0; i < numberThreads; i++) {
        if(pthread_join(threads[i], NULL)) {
            perror("Erro ao dar join das threads");
            exit(EXIT_FAILURE);
        }
    }
    print_tecnicofs_tree(fpO, fs);

    //Will save the end time of the the threads' execution.
    gettimeofday(&end, NULL);

    //Saves the time of the execution of the program.
    time = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec)/1000000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    if(pthread_mutex_destroy(&fLock)) exit(EXIT_FAILURE);

    free_tecnicofs(fs);
    free(threads);

    inode_table_destroy();



}
