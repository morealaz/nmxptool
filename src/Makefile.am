AUTOMAKE_OPTIONS = gnu
bin_PROGRAMS = nmxptool

# TODO optional
seedlinkplugin_sources = seedlink_plugin.c

nmxptool_SOURCES = nmxptool.c nmxptool_getoptlong.c nmxptool_ew.c \
		   $(seedlinkplugin_sources)

#noinst_HEADERS = gdu.h

# man page
# man_MANS = gdu.1x
#
# # we want these in the dist tarball
# EXTRA_DIST = back.xpm reload.xpm root.xpm \
#              home.xpm stop.xpm about.xpm gdu.1x
