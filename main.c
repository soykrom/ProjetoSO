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
#include "fs.h"
#include "macros.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

//Will contain the number of threads.
int numberThreads;

//Init of the pointer that'll point to the threads.
pthread_t *threads = NULL;

sem_t can_produce;
sem_t can_consume;

tecnicofs *fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName) {
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

int insertCommand(char* data) {
    //If the queue is full, it waits until it has space.
    if(sem_wait(&can_produce)) exit(EXIT_FAILURE);

    strcpy(inputCommands[numberCommands++], data);

    numberCommands = numberCommands % MAX_COMMANDS;

    if(sem_post(&can_consume)) exit(EXIT_FAILURE);
    return 1;
}

char* removeCommand() {
    char* command;
    //If the command is "@", it means the input has already finished.
    //There's no more commands after that.
    if(strcmp(inputCommands[headQueue], "@")) {

        command = inputCommands[headQueue++];
        headQueue = headQueue % MAX_COMMANDS;

        return command;
    }
    return NULL;
}

void errorParse(FILE *fp) {
    fprintf(fp, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void processInput(FILE *fp) {
    char line[MAX_INPUT_SIZE];

    while (fgets(line, sizeof(line)/sizeof(char), fp)) {

        char token;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) continue;

        switch (token) {
            case 'c':
            case 'l':
            case 'd':
                if(numTokens != 2) errorParse(fp);

                if(insertCommand(line)) break;
                return;
            case 'r':
                if(numTokens != 3) errorParse(fp);

                if(insertCommand(line)) break;
                return;
            case '#':
                break;
            default: { /* error */
                errorParse(fp);
            }
        }
    }
    fclose(fp);
}

void* applyCommands() {
    while(1) {
        if(sem_wait(&can_consume)) exit(EXIT_FAILURE);

        LOCK(&mLock);
        const char* command = removeCommand();

        //After a thread enters this condition, all of them will enter and exit the function.
        if (command == NULL) {
            UNLOCK(&mLock);
            if(sem_post(&can_consume)) exit(EXIT_FAILURE);
            return NULL;
        }

        char token;
        char name[MAX_INPUT_SIZE], newName[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, newName);
        int iNumber;

        //If the command is 'c', the iNumber is saved in order to prevent mixing up.
        if(token == 'c') iNumber = obtainNewInumber(fs);

        UNLOCK(&mLock);

        if(sem_post(&can_produce)) exit(EXIT_FAILURE);

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
    int i = 0;

    parseArgs(argc, argv);

    //Saves the number of threads.
    numberThreads =  atoi(argv[3]);
    if(numberThreads <= 0) {
        fprintf(stderr, "Error: invalid number of threads\n");
        exit(EXIT_FAILURE);
    }

    FILE *fpI = openFile(argv[1], "r");
    FILE *fpO = openFile(argv[2], "w");

    //Saves the number of buckets.
    nBuckets = atoi(argv[4]);

    threads = (pthread_t*) malloc(sizeof(pthread_t*) * numberThreads);

    fs = new_tecnicofs(nBuckets);

    if(sem_init(&can_produce, 0, 10)) exit(EXIT_FAILURE);
    if(sem_init(&can_consume, 0, 0)) exit(EXIT_FAILURE);

	create_locks(fs);

    //Will save the current time in 'start'.
    gettimeofday(&start, NULL);

    for(; i < numberThreads; i++)
        if(pthread_create(&threads[i], NULL, *applyCommands, NULL)) exit(EXIT_FAILURE);

    processInput(fpI);

    insertCommand("@");
    if(sem_post(&can_consume)) exit(EXIT_FAILURE);
    
    for(i = 0; i < numberThreads; i++) {
        if(pthread_join(threads[i], NULL)) exit(EXIT_FAILURE);
    }

    //Will save the end time of the the threads' execution.
    gettimeofday(&end, NULL);

    //Saves the time of the execution of the program.
    time = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_usec - start.tv_usec)/1000000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);
    free(threads);
    if(sem_destroy(&can_consume)) exit(EXIT_FAILURE);
    if(sem_destroy(&can_produce)) exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
