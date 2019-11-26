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

tecnicofs-rwlock: lib/hash.o lib/bst.o fs-rwlock.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o server-api lib/bst.o lib/hash.o fs-rwlock.o main-rwlock.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

lib/hash.o: lib/hash.c lib/hash.h
	$(CC) $(CFLAGS) -o lib/hash.o -c lib/hash.c

fs-rwlock.o: fs.c fs.h lib/bst.h lib/hash.h macros.h
	$(CC) $(CFLAGS) -DRWLOCK -o fs-rwlock.o -c fs.c

main-rwlock.o: main.c lib/bst.h lib/hash.h fs.h macros.h 
	$(CC) $(CFLAGS) -DRWLOCK -o main-rwlock.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-rwlock

clean-outputs:
	@echo Cleaning outputs...
	rm -f outputs/*.txt

clean-all: clean clean-outputs


#run: all jog clean

#jog:
#	./tecnicofs-rwlock inputs/test2.txt outputs/test3-rwlock.txt 4 5

#runt: all
#	./runTests.sh inputs/ outputs/ 4 4

#valgrind: all
#	valgrind --track-origins=yes ./tecnicofs-rwlock inputs/test1.txt outputs/test1-rwlock.txt 4 5