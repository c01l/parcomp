PROG=$1

if [ -z "$PROG" ]; then
	exit 1
fi

tmpf=$(mktemp)
tmpf2=$(mktemp)

i=0

$($PROG | grep "Time: " > $tmpf2)
while IFS='' read -r line || [[ -n "$line" ]]
do
	#echo $line
	
	regex=".* ([0-9]+)s ([0-9]+)ns$"
	if [[ $line =~ $regex ]]; then
		sec="${BASH_REMATCH[1]}"
		nsec="${BASH_REMATCH[2]}"
		
		printf "%s;%s;%s;\n" "$i" "$sec" "$nsec" >> $tmpf
		
	else 
		echo "Program gave the wrong output!"
	fi

	i=$((i+1))
done < "$tmpf2"


# find highest value in tmp file
sorted=$(cat $tmpf | sort --field-separator=";" -k 2 -k 3 -k 1 -n | tee $tmpf2)
#echo $sorted

used=$(tail -n 1 $tmpf2)
#echo $used

lineregex="[0-9]+;([0-9]+);([0-9]+)"
if [[ $used =~ $lineregex ]]; then
	sec="${BASH_REMATCH[1]}"
	nsec="${BASH_REMATCH[2]}"

	printf "Time: %ds %dns\n" "$sec" "$nsec"
else
	echo "Line has wrong format!!!"
fi

rm -f $tmpf $tmpf2