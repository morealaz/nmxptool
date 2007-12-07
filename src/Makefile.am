AUTOMAKE_OPTIONS = gnu
bin_PROGRAMS = nmxptool

nmxptool_SOURCES = nmxptool.c nmxptool_getoptlong.c nmxptool_ew.c

if WITHSEEDLINK
nmxptool_SOURCES += seedlink_plugin.c
endif

#noinst_HEADERS = gdu.h

# man page
# man_MANS = gdu.1x
#
# # we want these in the dist tarball
# EXTRA_DIST = back.xpm reload.xpm root.xpm \
#              home.xpm stop.xpm about.xpm gdu.1x
