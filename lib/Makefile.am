AUTOMAKE_OPTIONS = gnu

lib_LIBRARIES = libnmxp.a
libnmxp_a_CFLAGS = -I../include

INCDIR=../include
include_HEADERS = $(INCDIR)/nmxp.h \
		  $(INCDIR)/nmxp_base.h \
		  $(INCDIR)/nmxp_data.h \
		  $(INCDIR)/nmxp_chan.h \
		  $(INCDIR)/nmxp_log.h \
		  $(INCDIR)/nmxp_crc32.h

libnmxp_a_SOURCES = nmxp.c nmxp_base.c nmxp_data.c nmxp_chan.c nmxp_log.c nmxp_crc32.c


if WITHWINSOURCES
libnmxp_a_SOURCES += nmxp_win.c
include_HEADERS += $(INCDIR)/nmxp_win.h
endif


# libnmxp_la_LDFLAGS = -version-info 0:0:1

