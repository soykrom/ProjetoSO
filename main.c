/*Afonso Carvalho    ist193681
  João Gundersen     ist192494
  Grupo 15
*/

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
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "fs.h"
#include "macros.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100
#define MAX_CONNECTS 10

//Boolean to know if signal was triggered or not
int terminated = 1;

//Will contain the number of threads.
int numberThreads;
//Init of the pointer that'll point to the threads.
pthread_t *threads = NULL;

int numberCommands;
int headQueue;
//socket servidor
int sockfd;
struct sockaddr_un end_serv;

tecnicofs *fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

void signalHandler() {
    printf("\nTerminating");

    for(int  i = 0; i < numberThreads; i++) {
        if(pthread_join(threads[i], NULL)) {
            perror("Erro ao dar join das threads");
            exit(EXIT_FAILURE);
        }
    }

    terminated = 0;
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

int insertCommand(char* data) {
    //If the queue is full, it waits until it has space.

    strcpy(inputCommands[numberCommands++], data);

    numberCommands = numberCommands % MAX_COMMANDS;

    return 1;
}

char* removeCommand() {
    char* command;
    //If the command is "@", it means the input has already finished.
    //There's no more commands after that.
    if(strcmp(inputCommands[headQueue], "@")) {

        command = inputCommands[headQueue++];

        return command;
    }
    return NULL;
}

void errorParse(FILE *fp) {
    fprintf(fp, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void* clientHandler() {
    close(sockfd);

    printf("\nAm here\n");

    return NULL;
}

void* applyCommands() {
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
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
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
    double time;
    int nBuckets;
    int currentThread = 0;
    int dim_serv, dim_cli;
    struct sockaddr_un end_cli;

    parseArgs(argc, argv);

    if(signal(SIGINT, signalHandler) == SIG_ERR) {
        fprintf(stderr, "Error_ invalid signal handler");
        exit(EXIT_FAILURE);
    }

    FILE *fpO = openFile(argv[2], "w");

    nBuckets = atoi(argv[3]);
    if(nBuckets != 1) {
        perror("Error: invalid number of buckets");
        exit(EXIT_FAILURE);
    }

    fs = new_tecnicofs(nBuckets);

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

    listen(sockfd, MAX_CONNECTS);
    while(terminated) {
        int client_socket;

        dim_cli = sizeof(end_cli);

        client_socket = accept(sockfd, (struct sockaddr *) &end_serv, &dim_cli);
        if(client_socket < 0) {
            perror("Erro ao criar ligacao dedicada - accept");
            exit(EXIT_FAILURE);
        }

        if(pthread_create(&threads[currentThread], NULL, *clientHandler, NULL)) {
            perror("Erro ao criar thread para atender cliente");
            exit(EXIT_FAILURE);
        }
    }

    //Will save the end time of the the threads' execution.
    gettimeofday(&end, NULL);

    //Saves the time of the execution of the program.
    time = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec)/1000000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);
    free(threads);

    exit(EXIT_SUCCESS);
}
