for i in $(seq 1 4)
do
	export CILK_NWORKERS=$i
	./benchmark_allsizes.sh cilk_${i}cores cilk
done
echo "Finished"
