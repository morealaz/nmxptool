#!/bin/sh

# Syntax: cmd
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

echo "Begin: `date`" >> ${FILELOG}

FILE_STATION_LIST=`dirname $0`/nmxp_sds_station_list.conf
`dirname $0`/nmxp_stationlist.sh naqs2a.int.ingv.it data "\.HL[NEZ]" | sed -e "s/^/${DEFAULTNET}\./" > ${FILE_STATION_LIST} 2>> ${FILELOG}
STATION_LIST=`cat ${FILE_STATION_LIST}`

NMXP_SDS=`dirname $0`/nmxp_sds.sh 

JDAY=${JDAYSTART}
while [ $JDAY -le ${JDAYEND} ]; do
	echo "Download for year/jday ${YEAR}.${JDAY}" >> ${FILELOG} 2>&1
	for STATION in ${STATION_LIST}; do
		NET=`echo ${STATION} | cut -f 1 -d'.'`
		STA=`echo ${STATION} | cut -f 2 -d'.'`
		CHAN=`echo ${STATION} | cut -f 3 -d'.'`
		${NMXP_SDS} ${HOSTNAME} ${YEAR} ${NET} ${STA} ${CHAN} ${JDAY} ${OVERRIDE} >> ${FILELOG} 2>&1

	done
	JDAY=$(($JDAY + 1))
done

echo "End: `date`" >> ${FILELOG}

