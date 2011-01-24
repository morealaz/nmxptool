#!/bin/sh

. `dirname $0`/nmxp_sds.conf


# Syntax
SYNTAX="$0 <hostname> <'naqs'|'data'> [regexp_channel]"

HOSTNAME=$1
NAQSORDATA=$2
REGEXP=$3

if [ -z "${HOSTNAME}" ]; then
    echo ""
    echo "Syntax: ${SYNTAX}"
    echo ""
    exit
fi

if [ "${NAQSORDATA}" != "naqs" ] && [ ${NAQSORDATA} != "data" ]; then
    echo ""
    echo "Syntax: ${SYNTAX}"
    echo ""
    exit
fi

if [ "${NAQSORDATA}" == "naqs" ]; then
    NMXP_EXTRAOPTS="-L"
fi
if [ ${NAQSORDATA} == "data" ]; then
    NMXP_EXTRAOPTS="-l"
fi

$NMXPTOOL -H $HOSTNAME ${NMXP_EXTRAOPTS} | grep -E "${REGEXP}" | sed -e "s/[ ][ ]*/ /g" | cut -d' ' -f 3


