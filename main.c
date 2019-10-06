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
pthread_mutex_t lock; //So... many... bugs
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
    pthread_mutex_lock(&lock);

    const char* command = removeCommand();

    if (command == NULL) return NULL;

    char token;
    char name[MAX_INPUT_SIZE];
    int numTokens = sscanf(command, "%c %s", &token, name);


    printf("%c %s %d\n", token, name, numTokens);
    if (numTokens != 2) {
        fprintf(stderr, "Error: invalid command in Queue\n");
        exit(EXIT_FAILURE);
    }

    int searchResult;
    int iNumber;
    
    switch (token) {
        case 'c':
            iNumber = obtainNewInumber(fs);
            create(fs, name, iNumber);
            break;
        case 'l':
            searchResult = lookup(fs, name);
            if(!searchResult)
                fprintf(stderr, "%s not found\n", name);
            else
                fprintf(stderr, "%s found with inumber %d\n", name, searchResult);
            break;
        case 'd':
            delete(fs, name);
            break;
        default: { /* error */
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&lock);
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

    pthread_mutex_init(&lock, NULL);
    numberThreads =  atoi(argv[3]);
    threads = (pthread_t*) malloc(sizeof(pthread_t*) * numberThreads);

    parseArgs(argc, argv);

    fs = new_tecnicofs();
    processInput(fpI);

    for(; i < numberThreads; i++) {
        pthread_create(&threads[i], NULL, *applyCommands, NULL);
    }

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);

    start = clock() - start;
    time = (double) start / CLOCKS_PER_SEC;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    free(threads);
    exit(EXIT_SUCCESS);
}
