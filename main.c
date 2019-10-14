/*Afonso Carvalho    ist193681
  João Gundersen     ist192494
  Grupo 15
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "fs.h"


#ifdef MUTEX

/*Makefile replaces the variables with the corresponding code, creating an .exe with mutexs.*/
#define MUTEX_INIT(X) pthread_mutex_init(X, NULL)
#define MUTEX_LOCK(X) if(pthread_mutex_lock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define MUTEX_UNLOCK(X) if(pthread_mutex_unlock(X) != 0) {fprintf(stderr, "Error: unlocking failed"); exit(EXIT_FAILURE);}
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X)

#elif RWLOCK

/*Makefile replaces the variables with the corresponding code, creating an .exe with read_write_locks.*/
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define RWLOCK_INIT(X) pthread_rwlock_init(X, NULL)
#define RW_LOCK(X) if(pthread_rwlock_wrlock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define RD_LOCK(X) if(pthread_rwlock_rdlock(X) != 0) {fprintf(stderr, "Error: locking failed"); exit(EXIT_FAILURE);}
#define RW_UNLOCK(X) if(pthread_rwlock_unlock(X) != 0) {fprintf(stderr, "Error: unlocking failed"); exit(EXIT_FAILURE);}

#else

/*Makefile replaces the variables with the corresponding code, creating an .exe without sync.*/
#define MUTEX_LOCK(X)
#define MUTEX_UNLOCK(X)
#define RD_LOCK(X)
#define RW_LOCK(X)
#define RW_UNLOCK(X)
#endif

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

/*Will contain the number of threads.*/
int numberThreads;

pthread_mutex_t lockM;
pthread_mutex_t lockFS;
pthread_rwlock_t rwlockM;
pthread_rwlock_t rwlockFS;

/*Init of the pointer that'll point to the threads.*/
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

        /*If the command is 'c', the iNumber is saved in order to prevent mixing up.*/
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

/*If the creation of locks is unsuccessful, will exit the program.*/
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

    /*Creation of the time variables.*/
    struct timeval start, end;
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

    /*Will save the current time in 'start'.*/
    gettimeofday(&start, NULL);

    for(; i < numberThreads; i++) {
        pthread_create(&threads[i], NULL, *applyCommands, NULL);
    }

    for(i = 0; i < numberThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    /*Will save the end time of the the threads' execution.*/
    gettimeofday(&end, NULL);

    /*Saves the time of the execution of the program.*/
    time = (double) (end.tv_usec - start.tv_usec)/1000000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);

    print_tecnicofs_tree(fpO, fs);

    free_tecnicofs(fs);
    free(threads);

    exit(EXIT_SUCCESS);
}
