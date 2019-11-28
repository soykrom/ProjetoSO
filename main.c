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

//Will contain the number of threads.
int numberThreads;
int currentThread = 0;

//Init of the pointer that'll point to the threads.
pthread_t *threads = NULL;

//socket servidor
int sockfd;
int sockets[MAX_CONNECTS];
struct sockaddr_un end_serv;
struct ucred client_cred;

FILE *fpO;
tecnicofs *fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

void signalHandler() {
    printf("\nTerminating\n");

    if(close(sockfd) != 0)
        exit(EXIT_FAILURE);
    fprintf(fpO, "Hello\n");

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);
    free(threads);

    inode_table_destroy();

    exit(EXIT_SUCCESS);
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

void* clientHandler(void *uid) {
    char buffer[MAXLINHA + 1];
    int socket = sockets[currentThread - 1];
    int iNumber;
    int n;
    long cli_uid = atoi(uid);
    char token;
    int pos = 0;
    char name[MAX_INPUT_SIZE], otherInfo[MAX_INPUT_SIZE];

    while(1) {
        if(read(socket, buffer, MAXLINHA + 1) < 0)
            exit(EXIT_FAILURE);

        sscanf(buffer, "%c %s %s", &token, name, otherInfo);

        switch(token) {
            case 'c':
                LOCK(&locks[pos]);

                iNumber = inode_create(cli_uid, atoi(otherInfo)/10, atoi(otherInfo)%10);

                printf("O inumber de um novo ficheiro\n");
                printf("%d\n", iNumber);

                create(fs, name, iNumber, pos);

                strcpy(buffer, "1");

                n = strlen(buffer) + 1;
                if(write(socket, buffer, n) != n)
                    exit(EXIT_FAILURE);

                printf("A resposta ao cliente, que deve ser 1\n");
                printf("%s\n", buffer);

                UNLOCK(&locks[pos]);

                break;
            case 'd':
                LOCK(&locks[pos]);
                
                iNumber = lookup(fs, name, pos);

                delete(fs, name, pos);
                inode_delete(iNumber);

                printf("O inumber do ficheiro\n");
                printf("%d\n", iNumber);

                strcpy(buffer, "1");

                n = strlen(buffer) + 1;
                if(write(socket, buffer, n) != n)
                    exit(EXIT_FAILURE);

                UNLOCK(&locks[pos]);

            case 'e':
                strcpy(buffer, "1");

                n = strlen(buffer) + 1;
                if(write(socket, buffer, n) != n)
                    exit(EXIT_FAILURE);

                break;
        }

    }

    return NULL;
}

char* removeCommand() {
    return "suh";
}
/*void* applyCommands() {
    while(1) {

        LOCK(&mLock);
        const char* command = removeCommand();

        //After a thread enters this condition, all of them will enter and exit the function.
        if (command == NULL) {
            UNLOCK(&mLock);
            return NULL;
        }

        char token;
        char name[MAX_INPUT_SIZE], newName[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, newName);
        int iNumber;

        //If the command is 'c', the iNumber is saved in order to prevent mixing up.
        if(token == 'c') iNumber = obtainNewInumber(fs);

        UNLOCK(&mLock);


        if ((numTokens != 2 && token == 'r') && (token == 'r' && numTokens != 3)) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        int pos = hash(name, fs->nBuckets);

        switch (token) {
            case 'c':
                LOCK(&locks[pos]);

                create(fs, name, iNumber, pos);

                UNLOCK(&locks[pos]);
                break;
            case 'l':
                RD_LOCK(&locks[pos]);

                searchResult = lookup(fs, name, pos);
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);

                UNLOCK(&locks[pos]);
                break;
            case 'd':
                LOCK(&locks[pos]);

                delete(fs, name, pos);

                UNLOCK(&locks[pos]);
                break;
            case 'r':
                change_name(fs, name, newName, pos);

                break;
            default: {  error
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}*/

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

    if(unlink(argv[1]) != 0)
        exit(EXIT_FAILURE);

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

        client_socket = accept(sockfd, NULL, NULL);
        if(errno == EINTR) {
            printf("EINTR\n");
            break;
        }
        if(client_socket < 0) {
            perror("Erro ao criar ligacao dedicada - accept");
            exit(EXIT_FAILURE);
        }

        len_cred = sizeof(struct ucred);
        if(getsockopt(client_socket, SOL_SOCKET, SO_PEERCRED, &client_cred, &len_cred) == -1){
          perror("Erro ao obter as credenciais do cliente");
          exit(EXIT_FAILURE);
        }

        sprintf(uid, "%d", client_cred.uid);

        sockets[currentThread] = client_socket;
        if(pthread_create(&threads[currentThread++], NULL, *clientHandler, uid)) {
            perror("Erro ao criar thread para atender cliente");
            exit(EXIT_FAILURE);
        }
        numberThreads = currentThread;
    }

    printf("Hi\n");

    //Will save the end time of the the threads' execution.
    gettimeofday(&end, NULL);

    //Saves the time of the execution of the program.
    time = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec)/1000000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    for(int  i = 0; i < numberThreads; i++) {
        if(pthread_join(threads[i], NULL)) {
            perror("Erro ao dar join das threads");
            exit(EXIT_FAILURE);
        }
    }


}
