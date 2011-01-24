#!/bin/sh

DIRBASE=`dirname $0`

DIRFILECHANNEL=/home/sysop/seiscomp/acquisition
LISTFILECHANNEL="nmxp_channelfile_naqs1a.int.ingv.it.txt nmxp_channelfile_naqs1b.int.ingv.it.txt nmxp_channelfile_naqs2b.int.ingv.it.txt"
LISTTOL="NULL 10000 25000 50000"
YEAR=2008
JDAY=-1

FILEOUTPUT=/tmp/nmxp_sds_check_all.log

rm -f $FILEOUTPUT

echo "* * * BEGIN:  `date`" >> $FILEOUTPUT

for FILECHANNEL in ${LISTFILECHANNEL}; do
	for TOL in ${LISTTOL}; do
		echo "" >> $FILEOUTPUT
		echo ${FILECHANNEL} >> $FILEOUTPUT
		if [ ${TOL} == NULL ]; then
			TOL="        "
		fi
		$DIRBASE/nmxp_sds_check.sh ${DIRFILECHANNEL}/${FILECHANNEL} ${YEAR} ${JDAY} ${TOL} >> $FILEOUTPUT 2>&1
	done
done

echo "* * * END:  `date`" >> $FILEOUTPUT

cat $FILEOUTPUT | mail -s "nmxp_sds_check_all" quintiliani@ingv.it

