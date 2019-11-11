# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc -g
LD   = gcc -g
CFLAGS =-Wall -std=gnu99 -I../

LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
 .PHONY: all clean run

all: tecnicofs-mutex tecnicofs-rwlock tecnicofs-nosync

tecnicofs-mutex: lib/hash.o lib/bst.o fs-mutex.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-mutex lib/bst.o lib/hash.o fs-mutex.o main-mutex.o

tecnicofs-rwlock: lib/hash.o lib/bst.o fs-rwlock.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-rwlock lib/bst.o lib/hash.o fs-rwlock.o main-rwlock.o

tecnicofs-nosync: lib/hash.o lib/bst.o fs-nosync.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o lib/hash.o fs-nosync.o main-nosync.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

lib/hash.o: lib/hash.c lib/hash.h
	$(CC) $(CFLAGS) -o lib/hash.o -c lib/hash.c

fs-mutex.o: fs.c fs.h lib/bst.h lib/hash.h macros.h
	$(CC) $(CFLAGS) -DMUTEX -o fs-mutex.o -c fs.c

fs-rwlock.o: fs.c fs.h lib/bst.h lib/hash.h macros.h
	$(CC) $(CFLAGS) -DRWLOCK -o fs-rwlock.o -c fs.c

fs-nosync.o: fs.c fs.h lib/bst.h lib/hash.h macros.h
	$(CC) $(CFLAGS) -o fs-nosync.o -c fs.c

main-mutex.o: main.c lib/bst.h lib/hash.h fs.h macros.h
	$(CC) $(CFLAGS) -DMUTEX -o main-mutex.o -c main.c

main-rwlock.o: main.c lib/bst.h lib/hash.h fs.h macros.h
	$(CC) $(CFLAGS) -DRWLOCK -o main-rwlock.o -c main.c

main-nosync.o: main.c lib/bst.h lib/hash.h fs.h macros.h
	$(CC) $(CFLAGS) -o main-nosync.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-mutex tecnicofs-rwlock tecnicofs-nosync

clean-outputs:
	@echo Cleaning outputs...
	rm -f outputs/*.txt

clean-all: clean clean-outputs

run: all jog clean

jog:
	./tecnicofs-mutex inputs/test2.txt outputs/test3-mutex.txt 4 5
	./tecnicofs-rwlock inputs/test2.txt outputs/test3-rwlock.txt 4 5
	./tecnicofs-nosync inputs/test2.txt outputs/test3-nosync.txt 1 1

runt: all
	./runTests.sh inputs/ outputs/ 4 4

valgrind: all
	valgrind --track-origins=yes ./tecnicofs-mutex inputs/test1.txt outputs/test1-mutex.txt 4 5
	valgrind --track-origins=yes ./tecnicofs-rwlock inputs/test1.txt outputs/test1-rwlock.txt 4 5
	valgrind --track-origins=yes ./tecnicofs-nosync inputs/test1.txt outputs/test1-nosync.txt 1 1