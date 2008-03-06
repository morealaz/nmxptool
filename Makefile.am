# mtheo

SUBDIRS = src
dist_doc_DATA = BUGS HISTORY README earthworm seiscomp_templates makefile.sol makefile.ux

EWMAKEFILESOL=earthworm/makefile.ux.nognu
EWMAKEFILEUX=earthworm/makefile.ux.nognu

dist-hook:
	ln -f -s ${EWMAKEFILESOL} makefile.sol
	ln -f -s ${EWMAKEFILEUX} makefile.ux
	rm -rf `find $(distdir)/earthworm -name CVS`
	rm -rf `find $(distdir)/seiscomp_templates -name CVS`

binary_dist_FILES= \
	AUTHORS \
        BUGS \
        COPYING \
        HISTORY  \
        INSTALL \
        README  \
        README_DISTBIN \
        bin/nmxptool${EXEEXT} \
        earthworm/nmxptool.d \
        earthworm/nmxptool.desc \
	earthworm/nmxptool_cmd.html \
	earthworm/nmxptool_ovr.html \
        seiscomp_templates

binary_dist_name_suffix=${BINARY_DIST_NAME_SUFFIX}
binary_dist_name=${distdir}-bin-${binary_dist_name_suffix}

dist-bin:
	echo ${binary_dist_name}
	uname -a > README_DISTBIN
	rm -fr bin && mkdir bin && cp src/nmxptool${EXEEXT} bin/
	${AMTAR} chovf - --exclude "CVS" ${binary_dist_FILES} |  GZIP=$(GZIP_ENV) gzip -c > ${binary_dist_name}.tar.gz
#
# http://www.gnu.org/software/automake/manual/html_node/Extending.html#Extending
clean-local:
	@echo "example of clean-local"

NMXPTOOLBINNAME=nmxptool${EXEEXT}
NMXPTOOLBINSRC=src/${NMXPTOOLBINNAME}

DIRBINEW=${EW_HOME}/${EW_VERSION}/bin
NMXPTOOLBINEW=${DIRBINEW}/${NMXPTOOLBINNAME}

SEISCOMPDIRTEMPLATES=${SEISCOMPDIR}/acquisition/templates/source
DIRBINSEISCOMP=${SEISCOMPDIR}/acquisition/bin
NMXPTOOLBINSEISCOMP=${DIRBINSEISCOMP}/${NMXPTOOLBINNAME}

if WITHEW
install-ew-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINEW}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINEW}'

install-ew-conf:
	test -d "${EW_PARAMS}"
	${INSTALL_DATA} 'earthworm/nmxptool.d' '${EW_PARAMS}/nmxptool.d'
	${INSTALL_DATA} 'earthworm/nmxptool.desc' '${EW_PARAMS}/nmxptool.desc'

uninstall-ew-bin:
	rm -f ${NMXPTOOLBINEW}
endif

if WITHSEEDLINK
install-seiscomp-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINSEISCOMP}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINSEISCOMP}'

install-seiscomp-templates:
	test -d "${SEISCOMPDIRTEMPLATES}" 
	test -d "${SEISCOMPDIRTEMPLATES}/135_nmxptool"  || ./install-sh -c -d '${SEISCOMPDIRTEMPLATES}/135_nmxptool'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/chain_digiserv_station.tpl' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/chain_digiserv_station.tpl'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/chain_digiserv_station_trig.tpl' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/chain_digiserv_station_trig.tpl'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/key' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/key'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/plugins.ini.tpl' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/plugins.ini.tpl'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/seedlink_plugin.tpl' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/seedlink_plugin.tpl'
	${INSTALL_DATA} 'seiscomp_templates/135_nmxptool/seedlink_station.tpl' '${SEISCOMPDIRTEMPLATES}/135_nmxptool/seedlink_station.tpl'

uninstall-seiscomp-bin:
	rm -f ${NMXPTOOLBINSEISCOMP}
endif

