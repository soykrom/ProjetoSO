#!/bin/bash

if [ "$#" -eq 4 ] && [ -d $1 ] && [ -d $2 ] && [ $4 -gt 0 ]; 

then

	for filename in $1/*.txt; 
	
	do
		file=$(basename $filename)
		IFS='.' read -ra ADDR <<< "$file"

		echo InputFile=$file NumThreads=1
		./tecnicofs-nosync "$filename" "$2/$ADDR-1.txt" 1 1 | grep 'TecnicoFS'
	done

	if [ 2 -gt $3 ]; 
	
	then 
		exit
	fi

	for filename in $1/*.txt; 
	
	do
		file=$(basename $filename)
		IFS='.' read -ra ADDR <<< "$file"
		
		for ((threads = 2; threads <= $3; threads++)); 
		
		do
			echo InputFile=$file NumThreads=$threads

			./tecnicofs-mutex "$filename" "$2/$ADDR-$threads.txt" $threads $4 | grep 'TecnicoFS'
		done
	done
fi