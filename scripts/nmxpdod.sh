#!/bin/sh

############################################################################
# Syntax: nmxpdod host port ChannelList start_time duration maxwait nmxpwait
#	ChannelList STA1.CHAN1,STA1.CHAN2,STA2.CHAN1,...
#	start_time  YYYY/MM/DD,hh:mm:ss
#	duration    seconds
#	maxwait     seconds
#
# Author: Matteo Quintiliani - I.N.G.V. - quintiliani@ingv.it
############################################################################

if [ -z $7 ]; then
    echo "Syntax: nmxpdod host port ChannelList start_time duration maxwait nmxpwait"
    exit
fi

HOSTNAME=$1
PORT=$2
CHANNELIST=$3
STARTTIME=$4
DURATION=$5
MAXWAIT=$6
NMXPWAIT=$7

NOW=`date +%Y%m%dx%H%M%S`

FILEREQUEST=request.${NOW}.txt
NMXPTOOLBIN=../src/nmxptool

rm -f $FILEREQUEST

for c in `echo $CHANNELIST | sed -e "s/,/ /g"`; do
    echo `echo $c | sed -e "s/\./ /g"` `echo $STARTTIME | sed -e "s/[\/,]/-/g"` $DURATION $MAXWAIT >> $FILEREQUEST
done

# echo dod cli $FILEREQUEST $HOSTNAME:$PORT
# dod cli $FILEREQUEST $HOSTNAME:$PORT
echo java -cp /nmx/bin -ms5m ca.nanometrics.dataondemand.DataOnDemand cli $FILEREQUEST $HOSTNAME:$PORT
# java -cp /nmx/bin/DataOnDemand.jar -ms5m ca.nanometrics.dataondemand.DataOnDemand cli $FILEREQUEST $HOSTNAME:$PORT
java -cp /nmx/bin -ms5m ca.nanometrics.dataondemand.DataOnDemand cli $FILEREQUEST $HOSTNAME:$PORT

echo wait $NMXPWAIT seconds...
sleep ${NMXPWAIT}

echo $NMXPTOOLBIN -H $HOSTNAME -C $CHANNELIST -g -s $STARTTIME -t $DURATION
$NMXPTOOLBIN -H $HOSTNAME -C $CHANNELIST -g -s $STARTTIME -t $DURATION

