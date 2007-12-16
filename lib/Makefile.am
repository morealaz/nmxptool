AUTOMAKE_OPTIONS = gnu
lib_LIBRARIES = libnmxp.a

libnmxp_a_SOURCES = nmxp.c nmxp_base.c nmxp_data.c nmxp_chan.c nmxp_log.c nmxp_crc32.c

include_HEADERS =   nmxp.h nmxp_base.h nmxp_data.h nmxp_chan.h nmxp_log.h nmxp_crc32.h

if WITHWINSOURCES
libnmxp_a_SOURCES += nmxp_win.c
include_HEADERS += nmxp_win.h
endif

# libnmxp_la_LDFLAGS = -version-info 0:0:1

