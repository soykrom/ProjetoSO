#!/bin/bash

for filename in $1/*.txt; do
	file=$(basename $filename)
	IFS='.' read -ra ADDR <<< "$file"

	echo InputFile=$file NumThreads=1
	./tecnicofs-nosync "$filename" "$2/$ADDR-1.txt" 1 1 | grep 'TecnicoFS'
done

for filename in $1/*.txt; do
	file=$(basename $filename)
	IFS='.' read -ra ADDR <<< "$file"
	
	for ((threads = 2; threads <= $3; threads++)); do
		echo InputFile=$file NumThreads=$threads

		./tecnicofs-mutex "$filename" "$2/$ADDR-$threads.txt" $threads $4 | grep 'TecnicoFS'
	done


done
