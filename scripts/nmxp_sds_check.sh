#!/bin/sh

# Syntax: cmd <nmxptool_channelfile> <year> <jday>
# 
# TODO: add location
#
# Dependencies: bash, qmerge
#
# Author: Matteo Quintiliani - quintiliani@ingv.it - I.N.G.V.
#
#
# SDS structure: /YEAR/NET/STA/CHAN/  NET.STA.LOC.CHAN.YEAR.JDAY


export PATH="/bin:/usr/bin:/usr/local/bin:$PATH"

if [ -z $3 ]; then
	echo ""
	echo "Syntax: $0 <nmxptool_channelfile> <year> <jday>"
	echo ""
	exit
fi


NMXPCHANNELFILE=$1
YEAR=$2
JDAY=`echo $3 | sed -e "s/^[0]*//"`

# variables dependent on the input parameters

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

# add zero to JDAY
JDAY=$(printf %03d $JDAY)

#pseudo static variables
DIRARCHIVESDS=/home/sysop/seiscomp/acquisition/archive

# derivated variables
DIRLOG=$(dirname $0)/log
NOW=$(date "+%Y%m%dx%H%M%S")
QMERGE=qmerge

while read LINE
do
	NET=`echo $LINE | cut -d'.' -f 1`
	STA=`echo $LINE | cut -d'.' -f 2`
	CHANWILD=`echo $LINE | cut -d'.' -f 3`

	echo "$NET $STA $CHANWILD"

	if [ "$CHANWILD" == "HH?" ]; then
		CHANNELCOMPS="HHZ HHN HHE"
	else
		if [ "$CHANWILD" == "BH?" ]; then
			CHANNELCOMPS="BHZ BHN BHE"
		else
			CHANNELCOMPS=$CHANWILD
		fi
	fi

	for CHAN in $CHANNELCOMPS; do 
		FILEARCH=${DIRARCHIVESDS}/${YEAR}/${NET}/${STA}/${CHAN}.D/${NET}.${STA}.${LOC}.${CHAN}.D.${YEAR}.${JDAY}

		if [ -f ${FILEARCH} ]; then
			$QMERGE -n $FILEARCH
		else
			echo "WARNING: ${FILEARCH} does not exists!"
		fi
	done

done<$NMXPCHANNELFILE
