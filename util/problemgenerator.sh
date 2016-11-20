#!/bin/bash

if [ "$#" -ne "3" ]; then
	echo "Usage: $0 <sizeof-array1> <sizeof-array2> <output-file>"
	exit 1;
fi

sizeofArray1=$1
sizeofArray2=$2
output=$3

echo "Array 1 Size:" $sizeofArray1
echo "Array 2 Size:" $sizeofArray2
echo "Output file:" $output

prev=$(cat $output)
if [ -s "$output" ]; then
	echo "Output file is not empty... Exiting!"
	exit 2
fi

total=$(expr ${sizeofArray1} + ${sizeofArray2})
i=0

echo "Generating $total values..."
size1=0
size2=0

array1File=$(mktemp)
array2File=$(mktemp)

while [ "$size1" -lt "$sizeofArray1" ] && [ "$size2" -lt "$sizeofArray2" ]; do
	to=$(($RANDOM % 2))
	if [ "$to" -eq "0" ]; then
		echo "$i" >> $array1File
		size1=$((size1+1))
	else
		echo "$i" >> $array2File
		size2=$((size2+1))
	fi
	i=$((i+1))
done

while [ "$size1" -lt "$sizeofArray1" ]; do
	echo "$i" >> $array1File
	size1=$((size1+1))
	i=$((i+1))
done

while [ "$size2" -lt "$sizeofArray2" ];do
	echo "$i" >> $array2File
	size2=$((size2+1))
	i=$((i+1))
done

echo "Writing to $output..."

echo "$sizeofArray1" >> $output
cat $array1File >> $output
echo "$sizeofArray2" >> $output
cat $array2File >> $output

echo "Cleanup."

rm $array1File $array2File

echo "Done."