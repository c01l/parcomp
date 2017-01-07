
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

if [ "$OUTPUT" == "" ] || [ "$PROG" == "" ]; then
	echo "No output name specified!"
	echo
	
	usage
	exit 1
fi

# gather computer info
compinfo=$(lscpu)
compinfo_cpucount=$(echo "$compinfo" | grep 'CPU(s):' | cut -c9-)
compinfo_name=$(hostname)


printf "$compinfo_name\n\n" > $OUTPUT.txt

lscpu >> $OUTPUT.txt

printf "\n\n" >> $OUTPUT.txt

printf "Command: $PROG\n" >> $OUTPUT.txt
printf "Repetitions: $REPETITIONS\n\n" >> $OUTPUT.txt

benchinfo_starttime=$(date '+%F %H:%M:%S')
echo "Starting the benchmark at:" $benchinfo_starttime
echo "Benchmark started:" $benchinfo_starttime >> $OUTPUT.txt

printf "Run;Seconds;Nanoseconds\n" > $OUTPUT.csv

for i in $(seq $REPETITIONS)
do
	runoutput=$($PROG)
	echo "$runoutput" >> $OUTPUT.txt
	
	timeline=$(echo "$runoutput" | grep "Time:")
	echo $timeline
	
	regex=".* ([0-9]+)s ([0-9]+)ns$"
	if [[ $timeline =~ $regex ]]; then
		sec="${BASH_REMATCH[1]}"
		nsec="${BASH_REMATCH[2]}"
		
		printf "%s;%s;%s;\n" "$i" "$sec" "$nsec" >> $OUTPUT.csv
		
	else 
		echo "Program gave the wrong output!"
	fi
	
done;

benchinfo_endtime=$(date '+%F %H:%M:%S')
echo "Finished the benchmark at:" $benchinfo_endtime
echo "Benchmark ended:" $benchinfo_endtime >> $OUTPUT.txt

