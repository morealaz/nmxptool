#!/bin/sh

#
# Syntax: ./make_dist_bin.sh ewos ewarch
#
# Examples:
#		./make_dist_bin.sh macosx intel
#		./make_dist_bin.sh sol sparc
#

EWSOURCE=earthworm_sources_nmxptool
LIBMSEEDSOURCE=libmseed-2.1.4
NMXPSOURCE=nmxptool-1.2.2

EW_HOME="`pwd`"
EW_VERSION="$EWSOURCE"

if [ -z "$2" ]; then
    if [ -z "$1" ]; then
	cat GLOBALFLAGS
	echo ""
	echo "make_dist_bin.sh OS ARCH"
	echo ""
	exit
    else
	PREFIX="ew_${1}"
    fi
else
    PREFIX="ew_${1}_${2}"
fi

grep "${PREFIX}\." GLOBALFLAGS
RET=$?
GLOBALFLAGS_PRE=`grep "${PREFIX}\." GLOBALFLAGS | sed -e "s/^[^:][^:]*:[^\"][^\"]*//" -e "s/\"//g"`
if [ $RET -eq 0 ]; then
    EW_HOME_SUBS=`echo $EW_HOME | sed -e "s/\//|/g"`
    EW_VERSION_SUBS=`echo $EW_VERSION | sed -e "s/\//|/g"`
    GLOBALFLAGS=`echo $GLOBALFLAGS_PRE | sed -e "s/\\\${/X/g" -e "s/}/X/g" -e "s/XEW_HOMEX/${EW_HOME_SUBS}/g" -e "s/XEW_VERSIONX/${EW_VERSION_SUBS}/g" -e "s/|/\//g"`
    echo "GLOBALFLAGS=\"$GLOBALFLAGS\""
    ISOK=x
    echo -n "GLOBALFLAGS is ok? [y / n] "
    while [ $ISOK != "n" ] && [ $ISOK != "y" ]; do
	read ISOK
	echo ${ISOK}
    done
    if [ $ISOK != "y" ]; then
	echo "Abort"
	exit
    fi
else
    echo error
    exit
fi

for FSOURCE in $EWSOURCE $LIBMSEEDSOURCE $NMXPSOURCE; do
    gunzip < $FSOURCE.tar.gz | tar xvf -
done

cd libmseed
make
ranlib libmseed.a

CFLAGS="-O2 -Wall -pipe -I`pwd` ${CFLAGS_PTHREAD}"
LDFLAGS="-L`pwd` ${LDFLAGS_PTHREAD}"
LIBS="${LIBS_PTHREAD}"
cd ..

cd $NMXPSOURCE
echo "./configure CFLAGS=\"${CFLAGS}\" LDFLAGS=\"${LDFLAGS}\" EW_HOME=\"${EW_HOME}\" EW_VERSION=\"${EW_VERSION}\" GLOBALFLAGS=\"${GLOBALFLAGS}\""
./configure CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" LIBS="${LIBS}" EW_HOME="${EW_HOME}" EW_VERSION="${EW_VERSION}" GLOBALFLAGS="${GLOBALFLAGS}"
make
make dist-bin
src/nmxptool -V

cp *-bin-*.gz ../dist-bin/

cd ..

for FSOURCE in libmseed $EWSOURCE $NMXPSOURCE; do
    # rm -fr ${FSOURCE}
    echo "Leaving source directory ${FSOURCE}"
done

scp dist-bin/* mtheo@kyuzo.int.ingv.it:/Users/mtheo/Desktop/soft/unix_sources/nmxptool/

