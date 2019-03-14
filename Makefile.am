AM_DEFAULT_SOURCE_EXT = .cpp

if HAVE_SYSTEMD
systemdsystemunit_DATA = \
  gbmc-psu-hardreset.target
endif

noinst_LTLIBRARIES = libsyscmds_common.la
libsyscmds_common_la_SOURCES = cable.cpp cpld.cpp eth.cpp psu.cpp pcie_i2c.cpp entity_name.cpp
libsyscmds_common_la_CXXFLAGS = \
	$(SDBUSPLUS_CFLAGS) \
	$(PHOSPHOR_LOGGING_CFLAGS)
libsyscmds_common_la_LDFLAGS = \
	$(SDBUSPLUS_LIBS) \
	$(PHOSPHOR_LOGGING_LIBS) \
	-lstdc++fs

libsyscmdsdir = ${libdir}/ipmid-providers
libsyscmds_LTLIBRARIES = libsyscmds.la
libsyscmds_la_SOURCES = main.cpp
libsyscmds_la_LIBADD = libsyscmds_common.la
libsyscmds_la_LDFLAGS = \
	$(LIBIPMID_LIBS) \
	-version-info 0:0:0 -shared

libsyscmds_la_CXXFLAGS = \
	$(LIBIPMID_CFLAGS) \
	-flto

SUBDIRS = . test
