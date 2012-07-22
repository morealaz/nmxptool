#!/bin/bash

DIRNAME=`dirname $0`

# FROM_TAG=nmxptool_1_2_0
# TO_TAG=nmxptool_1_2_1

UFILE=${DIRNAME}/UFILE
HEADERFILE=${DIRNAME}/HEADERFILE

SVNURLNMXPTOOLTRUNK=svn+ssh://mtheo@svn.rm.ingv.it/svn/nmxptool/trunk
SVNAUTHORS=${DIRNAME}/SVNAUTHORS
CHANGELOGFILE=ChangeLog
CHANGELOGSVN2CLFILE=ChangeLogSVN2CL

date "+%Y-%m-%d %H:%M" > ${HEADERFILE}
echo "" >> ${HEADERFILE}
echo "	* nmxptool:" >> ${HEADERFILE}
echo "" >> ${HEADERFILE}
echo "	  Open-Source and Cross-Platform software for Nanometrics seismic data acquisition" >> ${HEADERFILE}
echo "	  Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy" >> ${HEADERFILE}
echo "	  Mail bug reports and suggestions to <matteo.quintiliani@ingv.it> " >> ${HEADERFILE}
echo "" >> ${HEADERFILE}
# echo "	  ChangeLog from tag ${FROM_TAG} to ${TO_TAG}" >> ${HEADERFILE}
# echo "" >> ${HEADERFILE}

# cat ${HEADERFILE} | cvs2cl.pl -r --delta ${FROM_TAG}:${TO_TAG} -U ${UFILE} -S --utc -I "ChangeLog" -I "^no_dist/*" --header -
# cat ${HEADERFILE} | cvs2cl.pl -r -U ${UFILE} -S --utc -I "ChangeLog" -I "^no_dist/*" --header -

# svn list -R | grep -v -E "^no_dist/" | grep -v ChangeLog > FILELIST
# svn2cl -a -i --authors=${SVNAUTHORS} --linelen=80 -f ${CHANGELOGSVN2CLFILE} ${SVNURLNMXPTOOLTRUNK} `cat FILELIST`
# svn2cl -a -i --authors=${SVNAUTHORS} --linelen=80 -f ${CHANGELOGSVN2CLFILE} ${SVNURLNMXPTOOLTRUNK} `cat FILELIST`
# svn log --verbose --xml svn+ssh://mtheo@svn.rm.ingv.it/svn/nmxptool/trunk `cat FILELIST` | xsltproc /usr/local/svn2cl/svn2cl.xsl - > TTTTTTTT

svn2cl -a -i --authors=${SVNAUTHORS} --linelen=80 -f ${CHANGELOGSVN2CLFILE}
cat ${HEADERFILE} ${CHANGELOGSVN2CLFILE} > ${CHANGELOGFILE}

