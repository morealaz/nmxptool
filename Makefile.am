# mtheo
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib src
dist_doc_DATA = BUGS HISTORY README.md earthworm/nmxptool_cmd.html earthworm/nmxptool_ovr.html

EWMAKEFILEUX=earthworm/makefile.ux.nognu

EXTRA_DIST = earthworm makefile.unix seiscomp2_templates seiscomp3_templates

dist-hook:
	ln -f -s ${EWMAKEFILEUX} makefile.unix

binary_distdir_FILES= \
	AUTHORS \
        BUGS \
        COPYING \
        HISTORY  \
        INSTALL \
        README.md  \
        README_DISTBIN \
        bin/nmxptool${EXEEXT} \
		earthworm \
		seiscomp2_templates \
		seiscomp3_templates

if ENABLE_DLL_PTHREAD
binary_distdir_FILES+= bin/${DLL_PTHREAD}
endif


binary_distdir_suffix=${BINARY_DIST_NAME_SUFFIX}
binary_distdir=${distdir}-bin-${binary_distdir_suffix}

TMPTARFILE=tmp_${binary_distdir}.tar

dist-bin:
	echo ${binary_distdir}
	uname -a > README_DISTBIN
	rm -fr bin && mkdir bin && cp src/nmxptool${EXEEXT} bin/
if ENABLE_DLL_PTHREAD
	test -f "${DIR_PTHREAD}/${DLL_PTHREAD}" && cp ${DIR_PTHREAD}/${DLL_PTHREAD} bin/
endif
	rm -fr ${binary_distdir}
	mkdir ${binary_distdir}
	${AMTAR} -cf ${TMPTARFILE} ${binary_distdir_FILES}
	cd ${binary_distdir} && ${AMTAR} xpf ../${TMPTARFILE}
	rm -f ${TMPTARFILE}
	rm -fr `find ${binary_distdir} -name CVS`
	rm -fr `find ${binary_distdir} -name .svn`
	${AMTAR} chvf - ${binary_distdir} |  GZIP=$(GZIP_ENV) gzip -c > ${binary_distdir}.tar.gz
	rm -fr ${binary_distdir}

# http://www.gnu.org/software/automake/manual/html_node/Extending.html#Extending
clean-local:
	@echo "example of clean-local"

NMXPTOOLBINNAME=nmxptool${EXEEXT}
NMXPTOOLBINSRC=src/${NMXPTOOLBINNAME}

DIRBINEW=${EW_HOME}/${EW_VERSION}/bin
NMXPTOOLBINEW=${DIRBINEW}/${NMXPTOOLBINNAME}

if ENABLE_EW
install-ew-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINEW}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINEW}'

install-ew-conf:
	test -d "${EW_PARAMS}"
	${INSTALL_DATA} 'earthworm/nmxptool.d' '${EW_PARAMS}/nmxptool.d'
	${INSTALL_DATA} 'earthworm/nmxptool.desc' '${EW_PARAMS}/nmxptool.desc'
	${INSTALL_DATA} 'earthworm/nmxptool.list.txt' '${EW_PARAMS}/nmxptool.list.txt'

install-ew-doc:
	test -d "${EW_HOME}/${EW_VERSION}/ewdoc/WEB_DOC/cmd"
	${INSTALL_DATA} 'earthworm/nmxptool_cmd.html' '${EW_HOME}/${EW_VERSION}/ewdoc/WEB_DOC/cmd/nmxptool_cmd.html'
	test -d "${EW_HOME}/${EW_VERSION}/ewdoc/WEB_DOC/ovr"
	${INSTALL_DATA} 'earthworm/nmxptool_ovr.html' '${EW_HOME}/${EW_VERSION}/ewdoc/WEB_DOC/ovr/nmxptool_ovr.html'

uninstall-ew-bin:
	rm -f ${NMXPTOOLBINEW}
endif

if ENABLE_SEEDLINK

### Seiscomp 3

# Based on ${SEISCOMPDIR}
SEISCOMP3DIRTEMPLATES=${SEISCOMPDIR}/share/templates/seedlink
DIRBINSEISCOMP3=${SEISCOMPDIR}/share/plugins/seedlink
NMXPTOOLBINSEISCOMP3=${DIRBINSEISCOMP3}/${NMXPTOOLBINNAME}

install-seiscomp3-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINSEISCOMP3}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINSEISCOMP3}'

install-seiscomp3-templates:
	test -d "${SEISCOMP3DIRTEMPLATES}" 
	test -d "${SEISCOMP3DIRTEMPLATES}/nmxp"  || ./install-sh -c -d '${SEISCOMP3DIRTEMPLATES}/nmxp'
	${INSTALL_DATA} 'seiscomp3_templates/nmxp/seedlink_plugin.tpl' '${SEISCOMP3DIRTEMPLATES}/nmxp/seedlink_plugin.tpl'

uninstall-seiscomp3-bin:
	rm -f ${NMXPTOOLBINSEISCOMP3}

### Seiscomp 2.5

# Based on ${SEISCOMPDIR}
SEISCOMP2DIRTEMPLATES=${SEISCOMPDIR}/acquisition/templates/source
DIRBINSEISCOMP2=${SEISCOMPDIR}/acquisition/bin
NMXPTOOLBINSEISCOMP2=${DIRBINSEISCOMP2}/${NMXPTOOLBINNAME}

install-seiscomp2-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINSEISCOMP2}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINSEISCOMP2}'

install-seiscomp2-templates:
	test -d "${SEISCOMP2DIRTEMPLATES}" 
	test -d "${SEISCOMP2DIRTEMPLATES}/135_nmxptool"  || ./install-sh -c -d '${SEISCOMP2DIRTEMPLATES}/135_nmxptool'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/chain_digiserv_station.tpl' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/chain_digiserv_station.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/chain_digiserv_station_trig.tpl' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/chain_digiserv_station_trig.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/key' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/key'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/plugins.ini.tpl' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/plugins.ini.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/seedlink_plugin.tpl' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/seedlink_plugin.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/135_nmxptool/seedlink_station.tpl' '${SEISCOMP2DIRTEMPLATES}/135_nmxptool/seedlink_station.tpl'
	test -d "${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod"  || ./install-sh -c -d '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/chain_digiserv_station.tpl' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/chain_digiserv_station.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/chain_digiserv_station_trig.tpl' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/chain_digiserv_station_trig.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/key' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/key'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/plugins.ini.tpl' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/plugins.ini.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/seedlink_plugin.tpl' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/seedlink_plugin.tpl'
	${INSTALL_DATA} 'seiscomp2_templates/136_nmxptool_dod/seedlink_station.tpl' '${SEISCOMP2DIRTEMPLATES}/136_nmxptool_dod/seedlink_station.tpl'

uninstall-seiscomp2-bin:
	rm -f ${NMXPTOOLBINSEISCOMP2}

endif

