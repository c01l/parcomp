
NAME=$1
PROG=$2

echo "Outputname: '$NAME'"
echo "Program: '$PROG'"

if [ -z "$NAME" ] || [ -z "$PROG" ]; then
	echo "Usage: $0 <outprefix> <programname>"
	exit 1
fi


sizes="50 100 1000 10000 100000 500000 1000000 5000000 10000000"
#sizes="50000000 100000000"
modes="r s1 s2 i"

for mode in $modes
do

	echo "Starting (mode: ${mode})..."
	for s in $sizes
	do
		./benchmark.sh "${NAME}_${mode}_$s" "./${PROG} -${mode} $s $s"
	done

	echo "Starting asymetric arrays (mode: ${mode})..."

	function asyncbench {
		./benchmark.sh "$1_async_$3_$4_$5" "./$2 -$3 $4 $5"
	}

	asyncbench "${NAME}" "${PROG}" "$mode" "150000" "50000"
	asyncbench "${NAME}" "${PROG}" "$mode" "50000" "150000"
	asyncbench "${NAME}" "${PROG}" "$mode" "1500000" "500000"
	asyncbench "${NAME}" "${PROG}" "$mode" "500000" "1500000"
done

echo "Done!"
