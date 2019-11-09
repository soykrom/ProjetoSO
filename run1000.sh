for((i = 0; i < 100; i++)); do
	./tecnicofs-mutex inputs/test1.txt outputs/test1-mutex.txt 4 5
	#valgrind --track-origins=yes ./tecnicofs-mutex inputs/test1.txt outputs/test1-mutex.txt 4 5
 done