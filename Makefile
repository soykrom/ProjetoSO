# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

CC   = gcc
LD   = gcc
CFLAGSMX =-Wall -DMUTEX -std=gnu99 -I../
CFLAGSWR =-Wall -DWRLOCK -std=gnu99 -I../
CFLAGSNS =-Wall -std=gnu99 -I../

LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs

tecnicofs: lib/bst.o fs.o main.o
	$(LD) $(CFLAGSMX) $(LDFLAGS) -o tecnicofs-mutex lib/bst.o fs.o main.o
	$(LD) $(CFLAGSWR) $(LDFLAGS) -o tecnicofs-rwlock lib/bst.o fs.o main.o
	$(LD) $(CFLAGSNS) $(LDFLAGS) -o tecnicofs-nosync lib/bst.o fs.o main.o

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

fs.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

main.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o main.o -c main.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs

run: tecnicofs
	./tecnicofs-mutex inputs/test1.txt outputs/test1.txt 10
	./tecnicofs-rwlock inputs/test1.txt outputs/test1.txt 10
	./tecnicofs-nosync inputs/test1.txt outputs/test1.txt 1