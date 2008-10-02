#!/bin/sh

# Syntax: cmd <hostname> <year> <net> <sta> <chan>  <jday> [y/n]
# 
#        Last parameter declare if override existing files. Default 'n'
#
# TODO: add location
#
# Dependencies: bash, stat (linux version), nmxptool
#
# Author: Matteo Quintiliani - quintiliani@ingv.it - I.N.G.V.
#
#
# SDS structure: /YEAR/NET/STA/CHAN/  NET.STA.LOC.CHAN.YEAR.JDAY

. `dirname $0`/nmxp_sds.conf

export PATH="/bin:/usr/bin:/usr/local/bin:$PATH"

if [ -z $6 ]; then
	echo ""
	echo "Syntax: $0 <hostname> <year> <net> <sta> <chan>  <jday>"
	echo "        if  <jday>  is less or equal 0,"
	echo "        it will be cosidered like a displacement from today."
	echo ""
	echo "Examples:"
	echo "        $0  naqs2a.int.ingv.it 2006 MN TIR HHZ 303"
	echo "        $0  naqs2a.int.ingv.it 2006 MN TIR HHZ  0 (today)"
	echo "        $0  naqs2a.int.ingv.it 2006 MN TIR HHZ -1 (yesterday)"
	echo "        $0  naqs2a.int.ingv.it 2006 MN TIR HHZ -7 (a week ago)"
	echo ""
	echo "WARNING: the displacement does not consider leap years if it refers to previous year !"
	echo ""
	exit
fi

# variables dependent on the input parameters
NMXPHOST=$1
YEAR=$2
NET=$3
STA=$4
CHAN=$5
JDAY=$6
JDAY=`echo $6 | sed -e "s/^[0]*//"`
OVERRIDE=$7

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

if [ "$CHAN" == "BH?" ]; then
	$0 $YEAR $NET $STA BHZ $JDAY
	$0 $YEAR $NET $STA BHN $JDAY
	$0 $YEAR $NET $STA BHE $JDAY
	exit
fi

if [ "$CHAN" == "HL?" ]; then
	$0 $YEAR $NET $STA HLZ $JDAY
	$0 $YEAR $NET $STA HLN $JDAY
	$0 $YEAR $NET $STA HLE $JDAY
	exit
fi

if [ "$CHAN" == "HN?" ]; then
	$0 $YEAR $NET $STA HNZ $JDAY
	$0 $YEAR $NET $STA HNN $JDAY
	$0 $YEAR $NET $STA HNE $JDAY
	exit
fi

if [ -z $OVERRIDE ]; then
	OVERRIDE=n
else
	if [ $OVERRIDE != "n" ] &&  [ $OVERRIDE != "y" ]; then
		echo "ERROR: Last parameter must be 'y' or 'n' !"
		exit
	fi
fi

if [ $OVERRIDE == "n" ]; then
	OVERRIDEMESSAGE="It will NOT BE overrode!"
else
	OVERRIDEMESSAGE="It will BE overrode!"
fi

# add zero to JDAY
JDAY=$(printf %03d $JDAY)

# derivated variables
DIRLOG=$(dirname $0)/log
NOW=$(date "+%Y%m%dx%H%M%S")
FILELOG=$DIRLOG/nmxp_sds.$1.$2.$3.$4.$5.$NOW.log
FILEMAILDAY=$DIRLOG/mail

FILEARCH=${DIRARCHIVESDS}/${YEAR}/${NET}/${STA}/${CHAN}.D/${NET}.${STA}.${LOC}.${CHAN}.D.${YEAR}.${JDAY}

# debugging variables
RUNNMXP=n

if [ -f ${FILEARCH} ]; then
	echo "WARNING: ${FILEARCH} already exists! ${OVERRIDEMESSAGE}"
	if [ $OVERRIDE == "n" ]; then
		RUNNMXP=n
	fi
fi

if [ $RUNNMXP == "y" ]; then
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
			mv -f ${NMXPFILEARCH} ${FILEARCH}
		fi
	else
		echo "ERROR: ${NMXPFILEARCH} has not been created by nmxptool."
	fi
fi

