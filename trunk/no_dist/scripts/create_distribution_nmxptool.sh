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
CVSTAGVERSION=`echo $DIRMODULE-$VERSION | sed -e "s/[\.-]/_/g"`

if [ -z $CVSWORK ]; then
    echo "WARNING: CVSWORK is not defined and will be set to the current directory!"
    CVSWORK=`pwd`
fi

CVSDIRBASE=$CVSWORK
CVSDIRMODULE=$CVSWORK/$DIRMODULE

if [ ! -d $CVSDIRMODULE ]; then
    echo "ERROR: $CVSDIRMODULE is not a directory!"
    exit
fi

echo "CVSTAGVERSION = $CVSTAGVERSION"

printf "Do you want execute 'cvs tag $CVSTAGVERSION' from directory $CVSDIRMODULE ? [ y/n ] "
EXECVSTAG=x
while [ $EXECVSTAG != y ]  &&  [ $EXECVSTAG != n ] && [ $EXECVSTAG != Y ]  &&  [ $EXECVSTAG != N ]; do
    read -s -n 1 -a EXECVSTAG
done
echo ""
echo "EXECVSTAG = $EXECVSTAG"
if [ $EXECVSTAG == y ]; then
    CURDIR=`pwd`
    cd $CVSDIRMODULE  &&  cvs tag $CVSTAGVERSION
    cd $CURDIR
fi

DIRTMP=/tmp/casa

rm -fr $DIRTMP

mkdir $DIRTMP

cd $DIRTMP

cvs export -r $CVSTAGVERSION $DIRMODULE  ||  exit

mv $DIRMODULE $DIRSOURCEVERSION

for DIRECTORY in $DIRSOURCEVERSION/libnmxp $DIRSOURCEVERSION ; do
    cd $DIRECTORY
    rm -fr `find . -name CVS`
    echo "Compiling $DIRECTORY"
    ./bootstrap
    mv configure configure.old
    cat configure.old | sed -e "s/^\(#define malloc rpl_malloc\)/\/\/ mtheo removed for solaris  \1/" > configure
    rm configure.old
    chmod 755 configure
    ./configure
    make
    cd -
done

for DIRECTORY in $DIRSOURCEVERSION/libnmxp $DIRSOURCEVERSION ; do
    cd $DIRECTORY
    echo "Cleaning $DIRECTORY"
    make clean
    make distclean
    rm bootstrap
    if [ -f Doxyfile ]; then
	doxygen
	rm Doxyfile
    fi
    cd -
done

# link to the earthworm makefiles
EWMAKEFILESOL=earthworm/makefile.ux.nognu
EWMAKEFILEUX=earthworm/makefile.ux.nognu
cd $DIRSOURCEVERSION
ln -s $EWMAKEFILESOL makefile.sol
ln -s $EWMAKEFILEUX makefile.ux
cd -


rm -f `find . -iname "*~"`
rm -fr $DIRSOURCEVERSION/libnmxp/doc/rapporto_tecnico_ingv_nmxp.*
rm -fr $DIRSOURCEVERSION/libnmxp/doc/nanometrics_naqs_and_data.graffle
rm -fr $DIRSOURCEVERSION/no_dist

tar cvfz $DIRSOURCEVERSION.tar.gz $DIRSOURCEVERSION

mv $DIRSOURCEVERSION.tar.gz $DIRBASE/

