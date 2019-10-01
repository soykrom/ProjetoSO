#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
tecnicofs *fs;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName) {
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]) {
    if (argc != 3) {
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
    if((numberCommands + 1)) {
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    return NULL;
}

void errorParse() {
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void processInput(const char* ficheiro) {
    char line[MAX_INPUT_SIZE];

    FILE *fp;

    fp = fopen(ficheiro, "r");

    if(fp == NULL) {
        printf("Erro a abrir ficheiro");
        return;
    }
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
                if(numTokens != 2) errorParse();

                if(insertCommand(line)) break;

                return;
            case '#':
                break;
            default: { /* error */
                errorParse();
            }
        }
    }
}

void applyCommands() {
    while(numberCommands > 0) {
        const char* command = removeCommand();

        if (command == NULL) continue;

        char token;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s", &token, name);

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
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                delete(fs, name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    clock_t start = clock();
    double time;

    parseArgs(argc, argv);

    fs = new_tecnicofs();
    processInput(argv[1]);
    applyCommands();

    print_tecnicofs_tree(argv[2], fs);

    free_tecnicofs(fs);

    start = clock() - start;
    time = (double) start / CLOCKS_PER_SEC;
    printf("TecnicoFS completed in %f seconds.\n", time);

    exit(EXIT_SUCCESS);
}
