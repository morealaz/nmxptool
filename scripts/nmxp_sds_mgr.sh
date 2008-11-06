#!/bin/sh

# Syntax: nmxp_sds_mgr.sh [ <year> <start_jday> [<end_jday>] ]
#                         [ <negative_start_jday> [<negative_end_jday>] ]
# 
# Dependencies: bash, nmxp_sds.sh
#
# TODO: input parameters
#
# Author: Matteo Quintiliani - quintiliani@ingv.it - I.N.G.V.
#

. `dirname $0`/nmxp_sds.conf

NOW=$(date "+%Y%m%dx%H%M%S")
FILELOG=`dirname $0`/nmxp_sds_mgr_${NOW}.log
NMXP_SDS=`dirname $0`/nmxp_sds.sh 

# compute year and julian day if julian day is defined less than zero
function jjday {
        YEAR="$1"
        JDAY="$2"
        NEWY=\$"$3"
        NEWJ=\$"$4"
	eval "$3=$YEAR"
        if [ $JDAY -le 0 ]; then
                export TODAY=$(date "+%j" | sed -e "s/^[0]*//")
                JDAY=$(( $TODAY + $JDAY ))
                eval "$4=$JDAY"
                if [ $JDAY -le 0 ]; then
                        echo "WARNING: the displacement does not consider leap years if it refers to previous year!" >> ${FILELOG}
			if [ $JDAY != 0 ]; then
				YEAR=$(( $YEAR - 1 ))
			fi
                        eval "$3=$YEAR"
                        JDAY=$(( 365 + $JDAY ))
                        eval "$4=$JDAY"
                        echo "WARNING: value for year and julian day are $YEAR/$JDAY!" >> ${FILELOG}
                fi
        fi
}

echo "Begin: `date`" >> ${FILELOG}

# if parameters are defined then they are overrode for YEAR, JDAYSTART and JDAYEND
if [ ! -z $1 ]; then
	if [ $1 -le 0 ]; then
		YEAR=`date +%Y`
		JDAYSTART=$1
		if [ -z $2 ]; then
			JDAYEND=$1
		else
			JDAYEND=$2
		fi
		jjday $YEAR $JDAYSTART YEARSTART JDAYSTART
		jjday $YEAR $JDAYEND   YEAREND JDAYEND
	else
		if [ ! -z $2 ]; then
			YEAR=$1
			YEARSTART=$1
			YEAREND=$1
			JDAYSTART=$2
			if [ -z $3 ]; then
				JDAYEND=$2
			else
				JDAYEND=$3
			fi
		else
			echo "Error in syntax. Exit" >> ${FILELOG}
			exit
		fi
	fi
fi


if [ ${YEARSTART} != ${YEAREND} ]; then
	echo "Error: start and end year are different! ${YEARSTART} ${YEAREND}" >> ${FILELOG}
	echo $YEARSTART.$JDAYSTART $YEAREND.$JDAYEND >> ${FILELOG}
	echo "Exit!" >> ${FILELOG}
	exit
else
	if [ ${YEARSTART} != ${YEAR} ]; then
		echo "Warning: year passed as argument is different from new computed year! ${YEAR} ${YEARSTART}" >> ${FILELOG}
		YEAR=$YEARSTART
	fi
fi

if [ ${JDAYSTART} -gt ${JDAYEND} ]; then
	echo "Error: start day is greater than end day! ${JDAYSTART} ${JDAYEND}" >> ${FILELOG}
	echo $YEARSTART.$JDAYSTART $YEAREND.$JDAYEND >> ${FILELOG}
	echo "Exit!" >> ${FILELOG}
	exit
fi

echo $YEARSTART.$JDAYSTART $YEAREND.$JDAYEND >> ${FILELOG}


FILE_STATION_LIST_TMP=`dirname $0`/nmxp_sds_station_list.conf.tmp
FILE_STATION_LIST=`dirname $0`/nmxp_sds_station_list.conf
`dirname $0`/nmxp_stationlist.sh naqs2a.int.ingv.it data "\.HL[NEZ]" | sed -e "s/^/${DEFAULTNET}\./" > ${FILE_STATION_LIST_TMP} 2>> ${FILELOG}
STATION_LIST=`cat ${FILE_STATION_LIST_TMP}`

rm -f ${FILE_STATION_LIST}
FIRST=1
for STATION in ${STATION_LIST}; do
	if [ ${FIRST} -eq 1 ]; then
		printf "${STATION}" >> ${FILE_STATION_LIST}
		FIRST=0
	else
		printf ",${STATION}" >> ${FILE_STATION_LIST}
	fi
done

JDAY=${JDAYSTART}
while [ $JDAY -le ${JDAYEND} ]; do
	echo "Download for year/jday ${YEAR}.${JDAY}" >> ${FILELOG} 2>&1
	echo ${NMXPTOOL} -H ${HOSTNAME} -m -C `cat ${FILE_STATION_LIST}` -s ${YEAR}.${JDAY},00:00:00.0000 -e  ${YEAR}.${JDAY},23:59:59.9999 -o ${DIRARCHIVESDS} >>  ${FILELOG} 2>&1
	${NMXPTOOL} -H ${HOSTNAME} -m -C `cat ${FILE_STATION_LIST}` -s ${YEAR}.${JDAY},00:00:00.0000 -e  ${YEAR}.${JDAY},23:59:59.9999 -o ${DIRARCHIVESDS} >>  ${FILELOG} 2>&1
# 	for STATION in ${STATION_LIST}; do
# 		NET=`echo ${STATION} | cut -f 1 -d'.'`
# 		STA=`echo ${STATION} | cut -f 2 -d'.'`
# 		CHAN=`echo ${STATION} | cut -f 3 -d'.'`
# 		${NMXP_SDS} ${HOSTNAME} ${YEAR} ${NET} ${STA} ${CHAN} ${JDAY} ${OVERRIDE} >> ${FILELOG} 2>&1
# 
# 	done
	JDAY=$(($JDAY + 1))
done

echo "End: `date`" >> ${FILELOG}

