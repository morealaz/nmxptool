# mtheo

SUBDIRS = src
dist_doc_DATA = README

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

if WITHEWSOURCES
install-ew-bin: ${NMXPTOOLBINSRC}
	test -d "${DIRBINEW}"
	${INSTALL_PROGRAM} '${NMXPTOOLBINSRC}' '${NMXPTOOLBINEW}'

install-ew-conf:
	test -d "${EW_PARAMS}"
	${INSTALL_DATA} 'earthworm/nmxptool.d' '${EW_PARAMS}/nmxptool.d'
	${INSTALL_DATA} 'earthworm/nmxptool.desc' '${EW_PARAMS}/nmxptool.desc'
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
endif

