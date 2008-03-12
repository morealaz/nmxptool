#!/bin/sh

# Syntax
SYNTAX="$0 <hostname> [regexp_channel]"

HOSTNAME=$1
REGEXP=$2


if [ -z "${HOSTNAME}" ]; then
    echo ""
    echo "Syntax: ${SYNTAX}"
    echo ""
    exit
fi

../src/nmxptool -H $HOSTNAME -L | grep -E "${REGEXP}" | sed -e "s/^.*        //"

# ../src/nmxptool -H $HOSTNAME -L | grep HZ | sed -e "s/^.*        //" -e "s/HZ/H?/g"

