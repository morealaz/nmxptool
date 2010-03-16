#!/bin/sh

############################################################################
# Syntax:
#
# Author: Matteo Quintiliani - I.N.G.V. - quintiliani@ingv.it
############################################################################

NMXPTOOLBIN=../../src/nmxptool
SLINKTOOLBIN=/Users/mtheo/Desktop/soft/unix_sources/src/slinktool/slinktool

DATASERVERS="naqs1a.int.ingv.it:28002 naqs2a.int.ingv.it:28002 naqs1b.int.ingv.it:28002 naqs2b.int.ingv.it:28002"
NAQSERVERS="naqs1a.int.ingv.it:28000 naqs2a.int.ingv.it:28000 naqs1b.int.ingv.it:28000 naqs2b.int.ingv.it:28000 naqs2b.int.ingv.it:26000"
SLSERVERS="hsl2.int.ingv.it:18000 gaia-cda1.int.ingv.it:18000 gaia-cda2.int.ingv.it:18000 gaia-cda3.int.ingv.it:18000 discovery.rm.ingv.it:39962"

echo "DELETE FROM cha_lnk_address WHERE id > 0;"

# NaqsServers
for NAQSSERVERPORT in ${NAQSERVERS}; do

    NAQSSERVER=`echo ${NAQSSERVERPORT} | cut -d':' -f 1`
    NAQSPORT=`echo ${NAQSSERVERPORT} | cut -d':' -f 2`

    ${NMXPTOOLBIN} -H ${NAQSSERVER} -P ${NAQSPORT} -L | sed -e "s/[ ][ ]*/ /g" | cut -f 3 -d' ' | tr '.' ' ' | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\)/CALL sp_update_channel_acquisition('', '\1', '\2', '', 0, '${NAQSSERVER}', ${NAQSPORT}, 'NaqsServer');/"
done


# DataServers
for DATASERVERPORT in ${DATASERVERS}; do

    DATASERVER=`echo ${DATASERVERPORT} | cut -d':' -f 1`
    DATAPORT=`echo ${DATASERVERPORT} | cut -d':' -f 2`

    ${NMXPTOOLBIN} -H ${DATASERVER} -D ${DATAPORT} -l | sed -e "s/[ ][ ]*/ /g" | cut -f 3 -d' ' | tr '.' ' ' | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\)/CALL sp_update_channel_acquisition('', '\1', '\2', '', 0, '${DATASERVER}', ${DATAPORT}, 'DataServer');/"
done


# SeedLink Servers
for SLSERVERPORT in ${SLSERVERS}; do

    SLSERVER=`echo ${SLSERVERPORT} | cut -d':' -f 1`
    SLPORT=`echo ${SLSERVERPORT} | cut -d':' -f 2`

    ${SLINKTOOLBIN} -Q ${SLSERVER}:${SLPORT} | grep " D " | gsed -r "~ s/^(.{9})([ ]{2})/\1--/" | sed -e "s/[ ][ ]*/ /g" | sed -e "s/\([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\) \([^ ][^ ]*\).*$/CALL sp_update_channel_acquisition('\1', '\2', '\4', '\3', 0, '${SLSERVER}', ${SLPORT}, 'SeedLink');/g"

done

