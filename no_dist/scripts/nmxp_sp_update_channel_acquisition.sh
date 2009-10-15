#!/bin/sh

############################################################################
# Syntax:
#
# Author: Matteo Quintiliani - I.N.G.V. - quintiliani@ingv.it
############################################################################

NMXPTOOLBIN=../../src/nmxptool
SLINKTOOLBIN=/Users/mtheo/Desktop/soft/unix_sources/src/slinktool/slinktool

DATASERVERS="naqs1a.int.ingv.it naqs2a.int.ingv.it naqs1b.int.ingv.it naqs2b.int.ingv.it"
NAQSERVERS="naqs1a.int.ingv.it naqs2a.int.ingv.it naqs1b.int.ingv.it naqs2b.int.ingv.it"
NAQSPORT=28000
DATAPORT=28002

SLSERVERS="hsl2.int.ingv.it gaia-cda1.int.ingv.it gaia-cda2.int.ingv.it gaia-cda3.int.ingv.it"
SLPORT=18000

for NAQSSERVER in ${NAQSERVERS}; do
    ${NMXPTOOLBIN} -H ${NAQSSERVER} -P ${NAQSPORT} -L | sed -e "s/[ ][ ]*/ /g" | cut -f 3 -d' ' | tr '.' ' ' | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\)/CALL sp_update_channel_acquisition('', '\1', '\2', '', 0, '${NAQSSERVER}', ${NAQSPORT}, 'NaqsServer');/"
done

for DATASERVER in ${DATASERVERS}; do
    ${NMXPTOOLBIN} -H ${DATASERVER} -D ${DATAPORT} -l | sed -e "s/[ ][ ]*/ /g" | cut -f 3 -d' ' | tr '.' ' ' | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\)/CALL sp_update_channel_acquisition('', '\1', '\2', '', 0, '${DATASERVER}', ${DATAPORT}, 'DataServer');/"
done


for SLSERVER in ${SLSERVERS}; do

    # BRUTAL calls for hsl1.int.ingv.it
    if [ "${SLSERVER}" == "hsl2.int.ingv.it" ]; then
	${SLINKTOOLBIN} -Q ${SLSERVER}:${SLPORT} | grep " D " | gsed -r "~ s/^(.{9})([ ]{2})/\1--/" | sed -e "s/[ ][ ]*/ /g" | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\).*$/CALL sp_update_channel_acquisition('\1', '\2', '\4', '\3', 0, 'hsl1.int.ingv.it', ${SLPORT}, 'SeedLink');/g"
    fi

    ${SLINKTOOLBIN} -Q ${SLSERVER}:${SLPORT} | grep " D " | gsed -r "~ s/^(.{9})([ ]{2})/\1--/" | sed -e "s/[ ][ ]*/ /g" | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\).*$/CALL sp_update_channel_acquisition('\1', '\2', '\4', '\3', 0, '${SLSERVER}', ${SLPORT}, 'SeedLink');/g"

done

