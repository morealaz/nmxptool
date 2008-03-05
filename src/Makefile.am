AUTOMAKE_OPTIONS = gnu
bin_PROGRAMS = nmxptool

nmxptool_SOURCES = nmxptool.c nmxptool_getoptlong.c nmxptool_ew.c

if WITHSEEDLINK
nmxptool_SOURCES += seedlink_plugin.c
endif

if WITHEWSOURCES
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/kom.c
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/getutil.c
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/logit.c
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/socket_ew_common.c
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/swap.c
nmxptool_SOURCES += @EW_LIBSRC_UTIL@/mem_circ_queue.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/transport.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/sleep_ew.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/socket_ew.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/time_ew.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/threads_ew.c
nmxptool_SOURCES += @EW_LIBSRC_OS@/sema_ew.c
endif

#noinst_HEADERS = gdu.h

# man page
# man_MANS = gdu.1x
#
# # we want these in the dist tarball
# EXTRA_DIST = back.xpm reload.xpm root.xpm \
#              home.xpm stop.xpm about.xpm gdu.1x
