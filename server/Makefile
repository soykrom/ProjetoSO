# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc -g
LD   = gcc -g
CFLAGS =-Wall -std=gnu99 -I../

LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
 .PHONY: all clean run

all: tecnicofs-rwlock

tecnicofs-rwlock: lib/hash.o lib/bst.o lib/inodes.o fs-rwlock.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o server-api lib/bst.o lib/hash.o lib/inodes.o fs-rwlock.o main-rwlock.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

lib/hash.o: lib/hash.c lib/hash.h
	$(CC) $(CFLAGS) -o lib/hash.o -c lib/hash.c

lib/inodes.o: lib/inodes.c lib/inodes.h
	$(CC) $(CFLAGS) -o lib/inodes.o -c lib/inodes.c

fs-rwlock.o: fs.c fs.h lib/bst.h lib/hash.h lib/inodes.h macros.h ../client/tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -DRWLOCK -o fs-rwlock.o -c fs.c

main-rwlock.o: main.c lib/bst.h lib/hash.h fs.h lib/inodes.h macros.h ../client/tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -DRWLOCK -o main-rwlock.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-rwlock

clean-outputs:
	@echo Cleaning outputs...
	rm -f outputs/*.txt

clean-all: clean clean-outputs


run: all jog clean

jog:
	./server-api /tmp/socket.unix.stream outputs/test1.txt 1 

runClient: client
	./client /tmp/socket.unix.stream

client:
	gcc client-api.c tecnicofs-client-api.c -o client
	

#runt: all
#	./runTests.sh inputs/ outputs/ 4 4

#valgrind: all
#	valgrind --track-origins=yes ./tecnicofs-rwlock inputs/test1.txt outputs/test1-rwlock.txt 4 5
