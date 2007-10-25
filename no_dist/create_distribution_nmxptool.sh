#!/bin/sh

if [ -z $1 ]; then
    echo "Syntax: `basename $0` <version> [<output_directory>]"
    exit
fi

VERSION=$1

if [ -z $2 ]; then
    DIRBASERELATIVE=`dirname $0`
    cd $DIRBASERELATIVE
    DIRBASE=`pwd`
    cd -
else
    if [ -d $2 ]; then
	DIRBASE=$2
    else
	echo "$2 is not a directory!"
	exit
    fi
fi

echo "DIRBASE = $DIRBASE"

DIRMODULE=nmxptool
DIRSOURCE=nmxptool
DIRSOURCEVERSION=$DIRSOURCE-$VERSION

DIRTMP=/tmp/casa

rm -fr $DIRTMP

mkdir $DIRTMP

cd $DIRTMP

cvs co $DIRMODULE

mv $DIRMODULE $DIRSOURCEVERSION

# rm -fr $DIRSOURCEVERSION/tools/nmxp_dap $DIRSOURCEVERSION/tools/nmxp_pds
rm -fr $DIRSOURCEVERSION/doc/rapporto_tecnico_ingv_nmxp.*
rm -fr $DIRSOURCEVERSION/doc/nanometrics_naqs_and_data.graffle

for DIRECTORY in $DIRSOURCEVERSION/libnmxp $DIRSOURCEVERSION ; do
    cd $DIRECTORY
    echo Cleaning $DIRECTORY
    rm -fr `find . -name CVS`
    ./bootstrap
    mv configure configure.old
    cat configure.old | sed -e "s/^\(#define malloc rpl_malloc\)/\/\/ mtheo removed for solaris  \1/" > configure
    chmod 755 configure
    ./configure
    make
    make clean
    make distclean
    rm missing bootstrap
    if [ -f Doxyfile ]; then
	doxygen
	rm Doxyfile
    fi
    cd -
done

rm -f `find . -iname "*~"`

tar cvfz $DIRSOURCEVERSION.tar.gz $DIRSOURCEVERSION

mv $DIRSOURCEVERSION.tar.gz $DIRBASE/

