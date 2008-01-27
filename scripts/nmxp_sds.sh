#!/bin/sh

# Syntax: cmd <year> <net> <sta> <chan>  <jday> 
# 
# TODO: add location

# SDS structure
# /YEAR/NET/STA/CHAN/
#		NET.STA.LOC.CHAN.YEAR.JDAY


export PATH="/bin:/usr/bin:/usr/local/bin:$PATH"

if [ -z $5 ]; then
	echo ""
	echo "Syntax: $0 <year> <net> <sta> <chan>  <jday>"
	echo "        if  <jday>  is less or equal 0,"
	echo "        it will be cosidered like a displacement from today."
	echo ""
	echo "Examples:"
	echo "        $0  2006 MN TIR HHZ 303"
	echo "        $0  2006 MN TIR HHZ  0 (today)"
	echo "        $0  2006 MN TIR HHZ -1 (yesterday)"
	echo "        $0  2006 MN TIR HHZ -7 (a week ago)"
	echo ""
	echo "WARNING: the displacement does not consider leap years if it refers to previous year !"
	echo ""
	exit
fi

# variables dependent on the input parameters
YEAR=$1
NET=$2
STA=$3
CHAN=$4.D
JDAY=$5

if [ $JDAY -le 0 ]; then
    	export TODAY=$(date "+%j" | sed -e "s/^[0]*//")
	export JDAY=$(( $TODAY + $JDAY ))
	if [ $JDAY -eq 0 ]; then
		echo ""
		echo "WARNING: the displacement does not consider leap years if it refers to previous year!"
		export JDAY=$(( 365 - $JDAY ))
		export YEAR=$(( $YEAR - 1 ))
		echo "WARNING: value for year and julian day are $YEAR/$JDAY!"
		echo ""
	fi
fi

if [ "$CHAN" == "HH?" ]; then
	$0 $YEAR $NET $STA HHZ $JDAY
	$0 $YEAR $NET $STA HHN $JDAY
	$0 $YEAR $NET $STA HHE $JDAY
	exit
fi

# add zero to JDAY
JDAY=$(printf %03d $JDAY)

#pseudo static variables
NMXPTOOL=nmxptool
NMXPHOST=naqs2a.int.ingv.it
DIRARCHIVESDS=/home/sysop/seiscomp/acquisition/archive
DIRARCHIVESDS2=/mnt/seedstore/archive

# derivated variables
DIRLOG=$(dirname $0)/log
NOW=$(date "+%Y%m%dx%H%M%S")
FILELOG=$DIRLOG/nmdc.$1.$2.$3.$4.$5.$NOW.log
FILEMAILDAY=$DIRLOG/mail

echo $NMXPTOOL -H $NMXPHOST -C ${STA}.${CHAN} -m
FILEARCH=${DIRARCHIVESDS}/${YEAR}/${NET}/${STA}/${CHAN}/${NET}.${STA}.${LOC}.${CHAN}.${YEAR}.${JDAY}
echo ${FILEARCH}


