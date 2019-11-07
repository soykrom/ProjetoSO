for filename in $1/*.txt; do
	file=$(basename $filename)
	IFS='.' read -ra ADDR <<< "$file"

	echo InputFile=$file NumThreads=1
	output=$(./tecnicofs-nosync "$filename" "$2/$ADDR-1.txt" 1 1)

	echo $output | sed 's/^.*TecnicoFS/TecnicoFS/'
done

for filename in $1/*.txt; do
	file=$(basename $filename)
	IFS='.' read -ra ADDR <<< "$file"
	
	for ((threads = 2; threads <= $3; threads++)); do
		echo InputFile=$file NumThreads=$threads

		output=$(./tecnicofs-mutex "$filename" "$2/$ADDR-$threads.txt" $threads $4)

		echo $output | sed 's/^.*TecnicoFS/TecnicoFS/'
	done


done
