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

tecnicofs-mutex: lib/bst.o fs.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-mutex lib/bst.o fs.o main-mutex.o

tecnicofs-rwlock: lib/bst.o fs.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-rwlock lib/bst.o fs.o main-rwlock.o

tecnicofs-nosync: lib/bst.o fs.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o fs.o main-nosync.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

fs.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

main-mutex.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -DMUTEX -o main-mutex.o -c main.c

main-rwlock.o: main.c fs.h lib/bst.h 
	$(CC) $(CFLAGS) -DRWLOCK -o main-rwlock.o -c main.c

main-nosync.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o main-nosync.o -c main.c


clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-mutex tecnicofs-rwlock tecnicofs-nosync

run: all
	./tecnicofs-mutex inputs/test1.txt outputs/test1.txt 10
	./tecnicofs-rwlock inputs/test1.txt outputs/test1.txt 10
	./tecnicofs-nosync inputs/test1.txt outputs/test1.txt 1

	./tecnicofs-mutex inputs/test2.txt outputs/test2.txt 10
	./tecnicofs-rwlock inputs/test2.txt outputs/test2.txt 10
	./tecnicofs-nosync inputs/test2.txt outputs/test2.txt 1

	./tecnicofs-mutex inputs/test3.txt outputs/test3.txt 10
	./tecnicofs-rwlock inputs/test3.txt outputs/test3.txt 10
	./tecnicofs-nosync inputs/test3.txt outputs/test3.txt 1

	./tecnicofs-mutex inputs/test4.txt outputs/test4.txt 10
	./tecnicofs-rwlock inputs/test4.txt outputs/test4.txt 10
	./tecnicofs-nosync inputs/test4.txt outputs/test4.txt 1