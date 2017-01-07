
NAME=$1
PROG=$2

if [ "$NAME"=="" ] || [ "$PROG"=="" ]; then
	echo "Usage: $0 <outprefix> <programname>"
fi


sizes="50 100 1000 10000 50000 100000 500000 1000000"

echo "Starting..."
for s in $sizes
do
	./benchmark.sh "${NAME}_$s" "./${PROG} -r $s $s"
done
echo "Done!"