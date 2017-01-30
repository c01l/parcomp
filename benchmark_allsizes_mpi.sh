
NAME=$1

echo "Outputname: '$NAME'"

if [ -z "$NAME" ]; then
	echo "Usage: $0 <outprefix>"
	exit 1
fi


sizes="60 120 10020 100020 500040 1000020 5000040 10000020"
#sizes="50000000 100000000"
modes="r s1 s2 i"

for cores in $(seq 1 4)
do
	for mode in $modes
	do

		echo "Starting (mode: ${mode})..."
		for s in $sizes
		do
			printf "Starting (cores=%s, mode=%s, size=%s)\n" "$cores" "$mode" "$s"
			./benchmark.sh "${NAME}_${cores}cores_${mode}_$s" "./benchmark_mpi.sh ${cores} ${mode} $s"
		done

		echo "Starting asymetric arrays (mode: ${mode})..."
	done
done
echo "Done!"
