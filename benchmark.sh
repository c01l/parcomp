
function usage {
	echo "Usage: $0 <name-of-the-benchmark> <program-name> [-r num]"
	echo
	echo "This script allowes to create statistics for a given program."
	echo "Generated files are:"
	echo " - <name-of-the-benchmark>.csv which contains the runtime-data"
	echo " - <name-of-the-benchmark>.info which contains additional information about the system that is used for the benchmark."
	echo
	echo "Parameters:"
	echo "   -r num"
	echo "        Specify the amount of repetitions, that are done for each test."
	echo "        Default: 35"
}

REPETITIONS=35
OUTPUT=""
PROG=""

# read parameters
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -r)
    REPETITIONS="$2"
    shift # past argument
    ;;
	
	-h|--help)
	usage
	exit 0
	;;
	
    *)
	if [ "$OUTPUT" == "" ]; then
		OUTPUT=$key
	elif [ "$PROG" == "" ]; then
		PROG=$key
	else
		echo "Unknown parameter: $key"
		echo 
		echo "Use $0 --help for further information!"
		exit 1
	fi
    ;;
esac
shift # past argument or value
done

if [ [ "$OUTPUT" == "" ] || [ "$PROG" == "" ] ]; then
	echo "No output name specified!"
	echo
	
	usage
	exit 1
fi

# gather computer info
compinfo=$(lscpu)
compinfo_cpucount=$(echo "$compinfo" | grep 'CPU(s):' | cut -c9-)
compinfo_name=$(hostname)

benchinfo_starttime=$(date '+%F %H:%M:%S')
echo "Starting the benchmark at:" $benchinfo_starttime

for i in $(seq $REPETITIONS)
do
	echo $("$PROG")
	
	# TODO extract information
done;

benchinfo_endtime=$(date '+%F %H:%M:%S')
echo "Finished the benchmark at:" $benchinfo_endtime


