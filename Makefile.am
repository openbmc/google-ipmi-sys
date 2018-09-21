AM_DEFAULT_SOURCE_EXT = .cpp

if HAVE_SYSTEMD
systemdsystemunit_DATA = \
  gbmc-psu-hardreset.target
endif

libsyscmdsdir = ${libdir}/ipmid-providers
libsyscmds_LTLIBRARIES = libsyscmds.la
libsyscmds_la_SOURCES = main.cpp cable.cpp cpld.cpp eth.cpp psu.cpp

libsyscmds_la_LDFLAGS = $(SYSTEMD_LIBS) \
                        $(SDBUSPLUS_LIBS) \
                        $(PHOSPHOR_LOGGING_LIBS) \
                        -lstdc++fs \
                        -version-info 0:0:0 -shared

libsyscmds_la_CXXFLAGS = $(SYSTEMD_CFLAGS) \
                         $(SDBUSPLUS_CFLAGS) \
                         $(PHOSPHOR_LOGGING_CFLAGS)

SUBDIRS = .
