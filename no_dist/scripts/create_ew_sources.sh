#!/bin/sh

if [ -z ${EW_VERSION} ] || [ -z ${EW_HOME} ] ; then
    echo "Error: EW_VERSION or EW_HOME are unset!"
    exit
fi

TARBASENAME=earthworm_sources_nmxptool
TARNAME=${TARBASENAME}.tar.gz

TMPDIR=/tmp/${TARBASENAME}
TMPTAR=/tmp/XXXXXXXXXX.tar.gz

EW_LIBSRC=$EW_HOME/$EW_VERSION/src/libsrc

rm -fr ${TMPDIR}
mkdir ${TMPDIR}


cp -R ${EW_HOME}/${EW_VERSION}/include ${TMPDIR}/
cp -R ${EW_HOME}/${EW_VERSION}/environment ${TMPDIR}/
cd $EW_LIBSRC
tar cvf - `find . -regex ".*\.[ch]"` | gzip -c > ${TMPTAR}
mkdir -p ${TMPDIR}/src/libsrc
cd ${TMPDIR}/src/libsrc
tar xvfz ${TMPTAR}
rm -f ${TMPTAR}

cd ${TMPDIR}/environment
for f in *.sh *.bash *.cmd; do
    SETCMD=`grep GLOBALFLAGS $f | grep -v "#" | grep -v "@" | cut -d' ' -f 1 | tail -n 1`
    if [ $SETCMD == export ]; then
	ASSIGN="="
    else
	ASSIGN=" "
    fi
    fup=../$f
    echo  "" 
    echo $f
    echo > $fup
    echo "cd \`dirname \$0\`" >> $fup
    echo "cd .." >> $fup
    echo "${SETCMD} EW_HOME${ASSIGN}\`pwd\`" >> $fup
    echo "${SETCMD} EW_VERSION${ASSIGN}${TARBASENAME}" >> $fup
    grep GLOBALFLAGS $f >> $fup
done

cd ${TMPDIR}
cd ..

tar cvfz ${TARNAME} ${TARBASENAME}

