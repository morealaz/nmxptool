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

STATION_LIST=`cat nmxp_sds_station_list.conf`
# ./nmxptool_filelist.sh naqs2a.int.ingv.it data "\.HL[NEZ]"

NMXP_SDS=`dirname $0`/nmxp_sds.sh 

JDAY=${JDAYSTART}
while [ $JDAY -le ${JDAYEND} ]; do
	echo "Download for year/jday ${YEAR}.${JDAY}"
	for STATION in ${STATION_LIST}; do
		NET=`echo ${STATION} | cut -f 1 -d'.'`
		STA=`echo ${STATION} | cut -f 2 -d'.'`
		CHAN=`echo ${STATION} | cut -f 3 -d'.'`
		echo ${NMXP_SDS} ${HOSTNAME} ${YEAR} ${NET} ${STA} ${CHAN} ${JDAY} ${OVERRIDE}
	done
	JDAY=$(($JDAY + 1))
done
