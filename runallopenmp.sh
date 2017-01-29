for i in $(seq 1 4)
do
	echo "Using $i threads now..."
	export OMP_NUM_THREADS=$i
	./benchmark_allsizes.sh "openmp_${i}cores" "openmp"
done
echo "Finished"
