#!/bin/sh

# Syntax: cmd <year> <net> <sta> <chan>  <jday> 
# 
# TODO: add location
#
# Dependencies: bash, stat (linux version), nmxptool
#
# Author: Matteo Quintiliani - quintiliani@ingv.it - I.N.G.V.
#
#
# SDS structure: /YEAR/NET/STA/CHAN/  NET.STA.LOC.CHAN.YEAR.JDAY


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
CHAN=$4
JDAY=$5
JDAY=`echo $5 | sed -e "s/^[0]*//"`
NMXPHOST=naqs2a.int.ingv.it

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
NMXPBINDIR=/home/sysop/seiscomp/acquisition/bin
NMXPTOOL=${NMXPBINDIR}/nmxptool
DIRARCHIVESDS=/mnt/seedstore/nmxp_accel

# derivated variables
DIRLOG=$(dirname $0)/log
NOW=$(date "+%Y%m%dx%H%M%S")
FILELOG=$DIRLOG/nmdc.$1.$2.$3.$4.$5.$NOW.log
FILEMAILDAY=$DIRLOG/mail

FILEARCH=${DIRARCHIVESDS}/${YEAR}/${NET}/${STA}/${CHAN}.D/${NET}.${STA}.${LOC}.${CHAN}.D.${YEAR}.${JDAY}

if [ -f ${FILEARCH} ]; then
	echo "WARNING: ${FILEARCH} already exists! It will not be override!"
else
	$NMXPTOOL -H $NMXPHOST -m -C ${NET}.${STA}.${CHAN} -s ${YEAR}.${JDAY},00:00:00.0000 -e  ${YEAR}.${JDAY},23:59:59.9999
	NMXPFILEARCH=${NET}.${STA}.${CHAN}_${YEAR}.${JDAY}.00.00.00.0000_${YEAR}.${JDAY}.23.59.59.9999.miniseed
	if [ -f $NMXPFILEARCH ]; then
		FILENMXPARCHSIZE=`stat -c %s ${NMXPFILEARCH}`
		if [ $FILENMXPARCHSIZE -le 0 ]; then
			echo "WARNING: ${NMXPFILEARCH} is empty! It will not be saved!"
			rm -f $NMXPFILEARCH
		else
			echo "Moving ${NMXPFILEARCH} to ${FILEARCH} ... "
			mkdir -p `dirname ${FILEARCH}`
			mv ${NMXPFILEARCH} ${FILEARCH}
		fi
	else
		echo "ERROR: ${NMXPFILEARCH} has not been created by nmxptool."
	fi
fi

