# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../

LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
 .PHONY: all clean run

all: tecnicofs-mutex tecnicofs-rwlock tecnicofs-nosync

tecnicofs-mutex: lib/hash.o lib/bst.o locks-mutex.o fs.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-mutex lib/bst.o lib/hash.o locks-mutex.o fs.o main-mutex.o

tecnicofs-rwlock: lib/hash.o lib/bst.o locks-rwlock.o fs.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-rwlock lib/bst.o lib/hash.o locks-rwlock.o fs.o main-rwlock.o

tecnicofs-nosync: lib/hash.o lib/bst.o locks-nosync.o fs.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o lib/hash.o locks-nosync.o fs.o main-nosync.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

lib/hash.o: lib/hash.c lib/hash.h
	$(CC) $(CFLAGS) -o lib/hash.o -c lib/hash.c

locks-mutex.o: locks.c locks.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) -DMUTEX -o locks-mutex.o -c locks.c

locks-rwlock.o: locks.c locks.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) -DRWLOCK -o locks-rwlock.o -c locks.c

locks-nosync.o: locks.c locks.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) -o locks-nosync.o -c locks.c

fs.o: fs.c fs.h lib/bst.h lib/hash.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

main-mutex.o: main.c lib/bst.h lib/hash.h locks.h fs.h 
	$(CC) $(CFLAGS) -DMUTEX -o main-mutex.o -c main.c

main-rwlock.o: main.c lib/bst.h lib/hash.h locks.h fs.h 
	$(CC) $(CFLAGS) -DRWLOCK -o main-rwlock.o -c main.c

main-nosync.o: main.c lib/bst.h lib/hash.h locks.h fs.h 
	$(CC) $(CFLAGS) -o main-nosync.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-mutex tecnicofs-rwlock tecnicofs-nosync

clean-outputs:
	@echo Cleaning...
	rm -f outputs/*.txt

run: all jog clean

jog:
	./tecnicofs-mutex inputs/test3.txt outputs/test3-mutex.txt 10 5
	./tecnicofs-rwlock inputs/test3.txt outputs/test3-rwlock.txt 10 5
	./tecnicofs-nosync inputs/test3.txt outputs/test3-nosync.txt 1 1

valgrind: all
	valgrind --leak-check=full ./tecnicofs-mutex inputs/test1.txt outputs/test1-mutex.txt 10 4
	valgrind --leak-check=full ./tecnicofs-rwlock inputs/test1.txt outputs/test1-rwlock.txt 10 4
	valgrind --leak-check=full ./tecnicofs-nosync inputs/test1.txt outputs/test1-nosync.txt 1 1