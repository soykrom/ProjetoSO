#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "fs.h"


#ifdef MUTEX
#define MUTEX_INIT(X) pthread_mutex_init(X, NULL)
#define MUTEX_LOCK(X) if(pthread_mutex_lock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define MUTEX_UNLOCK(X) if(pthread_mutex_unlock(X) != 0) {fprintf(stderr, "Error: unlocking failed"); exit(EXIT_FAILURE);}
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X) 

#elif RWLOCK
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define RWLOCK_INIT(X) pthread_rwlock_init(X, NULL)
#define RW_LOCK(X) if(pthread_rwlock_wrlock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define RD_LOCK(X) if(pthread_rwlock_rdlock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define RW_UNLOCK(X) if(pthread_rwlock_unlock(X) != 0) {fprintf(stderr, "Error: unlocking failed"); exit(EXIT_FAILURE);}

#else
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X)
#endif

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100


int numberThreads = 0;
pthread_mutex_t lockM;
pthread_mutex_t lockFS;
pthread_rwlock_t rwlockM;
pthread_rwlock_t rwlockFS;
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
    if((numberCommands > 0)) {
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
        MUTEX_LOCK(&lockM);
        RW_LOCK(&rwlockM);

        const char* command = removeCommand();
        
        if (command == NULL) {
            MUTEX_UNLOCK(&lockM);
            RW_UNLOCK(&rwlockM);
            return NULL;
        }

        char token;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s", &token, name);
        int iNumber;

        if(token == 'c') iNumber = obtainNewInumber(fs);

        MUTEX_UNLOCK(&lockM);
        RW_UNLOCK(&rwlockM);

        if (numTokens != 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        
        switch (token) {
            case 'c':   
                MUTEX_LOCK(&lockFS);
                RW_LOCK(&rwlockFS);

                create(fs, name, iNumber);

                MUTEX_UNLOCK(&lockFS);
                RW_UNLOCK(&rwlockFS);
                break;
            case 'l':
                MUTEX_LOCK(&lockFS);
                RD_LOCK(&rwlockFS);
                searchResult = lookup(fs, name);
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);

                MUTEX_UNLOCK(&lockFS);
                RW_UNLOCK(&rwlockFS);
                break;
            case 'd':
                MUTEX_LOCK(&lockFS);
                RW_LOCK(&rwlockFS);

                delete(fs, name);

                MUTEX_UNLOCK(&lockFS);
                RW_UNLOCK(&rwlockFS);
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

void create_locks() {
    #ifdef MUTEX
    if(pthread_mutex_init(&lockM, NULL) != 0 || pthread_mutex_init(&lockFS, NULL) != 0){
        fprintf(stderr, "Error: lock creation failed");
        exit(EXIT_FAILURE);
    }

    #elif RWLOCK
    if(pthread_rwlock_init(&rwlockM, NULL) != 0 || pthread_rwlock_init(&rwlockFS, NULL) != 0) {
        fprintf(stderr, "Error: lock creation failed");
        exit(EXIT_FAILURE);
    }
    #endif
}

int main(int argc, char *argv[]) {
    clock_t start;
    double time;
    FILE *fpI = openFile(argv[1], "r"); 
    FILE *fpO = openFile(argv[2], "w");
    int i = 0;

    numberThreads =  atoi(argv[3]);
    threads = (pthread_t*) malloc(sizeof(pthread_t*) * numberThreads);

    parseArgs(argc, argv);

    fs = new_tecnicofs();
    processInput(fpI);

    create_locks();

    start = clock();

    for(; i < numberThreads; i++) {
        pthread_create(&threads[i], NULL, *applyCommands, NULL);
    }

    for(i = 0; i < numberThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    start = clock() - start;
    time = (double) start / CLOCKS_PER_SEC;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);
    
    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);

    free(threads);
    exit(EXIT_SUCCESS);
}

