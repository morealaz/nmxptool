#!/bin/bash

DIRNAME=`dirname $0`

# FROM_TAG=nmxptool_1_2_0
# TO_TAG=nmxptool_1_2_1

UFILE=${DIRNAME}/UFILE
HEADERFILE=${DIRNAME}/HEADERFILE

date "+%Y-%m-%d %H:%M" > ${HEADERFILE}
echo "" >> ${HEADERFILE}
echo "	* nmxptool:" >> ${HEADERFILE}
echo "" >> ${HEADERFILE}
echo "	  Open-Source and Cross-Platform software for Nanometrics seismic data acquisition" >> ${HEADERFILE}
echo "	  Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy" >> ${HEADERFILE}
echo "	  Mail bug reports and suggestions to <quintiliani@ingv.it> " >> ${HEADERFILE}
echo "" >> ${HEADERFILE}
# echo "	  ChangeLog from tag ${FROM_TAG} to ${TO_TAG}" >> ${HEADERFILE}
# echo "" >> ${HEADERFILE}

# cat ${HEADERFILE} | cvs2cl.pl -r --delta ${FROM_TAG}:${TO_TAG} -U ${UFILE} -S --utc -I "ChangeLog" -I "^no_dist/*" --header -
cat ${HEADERFILE} | cvs2cl.pl -r -U ${UFILE} -S --utc -I "ChangeLog" -I "^no_dist/*" --header -

