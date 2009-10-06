#!/bin/sh

############################################################################
# Syntax:
#
# Author: Matteo Quintiliani - I.N.G.V. - quintiliani@ingv.it
############################################################################

NMXPTOOLBIN=../src/nmxptool

NAQSERVERS="naqs1a.int.ingv.it naqs2a.int.ingv.it naqs1b.int.ingv.it naqs2b.int.ingv.it"
NAQSPORT=28000

for NAQSSERVER in ${NAQSERVERS}; do
    ${NMXPTOOLBIN} -H ${NAQSSERVER} -P ${NAQSPORT} -L | sed -e "s/[ ][ ]*/ /g" | cut -f 3 -d' ' | tr '.' ' ' | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\)/sp_update_channel_acquisition('\1', '\2', 0, '${NAQSSERVER}', ${NAQSPORT});/"
done

