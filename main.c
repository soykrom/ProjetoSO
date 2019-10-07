#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
pthread_mutex_t lockM;
pthread_mutex_t lockFS;
pthread_t *threads = NULL;

tecnicofs *fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

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
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if((numberCommands)) {
        numberCommands--;
        return inputCommands[headQueue++];  
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
    while(numberCommands > 0) {
        pthread_mutex_lock(&lockM);
        const char* command = removeCommand();
        
        if (command == NULL) {
            pthread_mutex_unlock(&lockM);
            return NULL;
        }

        char token;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s", &token, name);
        int iNumber;

        if(token == 'c') iNumber = obtainNewInumber(fs);
        pthread_mutex_unlock(&lockM);

        if (numTokens != 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        
        switch (token) {
            case 'c':
                pthread_mutex_lock(&lockFS);
                create(fs, name, iNumber);
                pthread_mutex_unlock(&lockFS);
                break;
            case 'l':
                pthread_mutex_lock(&lockFS);
                searchResult = lookup(fs, name);
                if(!searchResult)
                    fprintf(stderr, "%s not found\n", name);
                else
                    fprintf(stderr, "%s found with inumber %d\n", name, searchResult);
                pthread_mutex_unlock(&lockFS);
                break;
            case 'd':
                pthread_mutex_lock(&lockFS);
                delete(fs, name);
                pthread_mutex_unlock(&lockFS);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
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
    clock_t start = clock();
    double time;
    FILE *fpI = openFile(argv[1], "r"); 
    FILE *fpO = openFile(argv[2], "w");
    int i = 0;

    pthread_mutex_init(&lockM, NULL);
    pthread_mutex_init(&lockFS, NULL);

    numberThreads =  atoi(argv[3]);
    threads = (pthread_t*) malloc(sizeof(pthread_t*) * numberThreads);

    parseArgs(argc, argv);

    fs = new_tecnicofs();
    processInput(fpI);

    for(; i < numberThreads; i++) {
        pthread_create(&threads[i], NULL, *applyCommands, NULL);
    }

    for(i = 0; i < numberThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);

    start = clock() - start;
    time = (double) start / CLOCKS_PER_SEC;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    free(threads);
    exit(EXIT_SUCCESS);
}