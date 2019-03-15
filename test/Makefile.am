AM_CPPFLAGS = \
	-I$(top_srcdir)/ \
	$(GTEST_CFLAGS) \
	$(GMOCK_CFLAGS) \
	$(CODE_COVERAGE_CPPFLAGS)
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)
AM_CXXFLAGS = $(CODE_COVERAGE_CXXFLAGS)
AM_LDFLAGS = \
	$(GTEST_LIBS) \
	$(GMOCK_LIBS) \
	-lgmock_main \
	$(OESDK_TESTCASE_FLAGS) \
	$(CODE_COVERAGE_LIBS)

check_PROGRAMS =
TESTS = $(check_PROGRAMS)

check_PROGRAMS += handler_unittest
handler_unittest_SOURCES = handler_unittest.cpp
handler_unittest_LDADD = $(top_builddir)/libsyscmds_common.la

check_PROGRAMS += eth_unittest
eth_unittest_SOURCES = eth_unittest.cpp
eth_unittest_LDADD = $(top_builddir)/libsyscmds_common.la

check_PROGRAMS += cable_unittest
cable_unittest_SOURCES = cable_unittest.cpp
cable_unittest_LDADD = $(top_builddir)/libsyscmds_common.la

check_PROGRAMS += cpld_unittest
cpld_unittest_SOURCES = cpld_unittest.cpp
cpld_unittest_LDADD = $(top_builddir)/libsyscmds_common.la

check_PROGRAMS += psu_unittest
psu_unittest_SOURCES = psu_unittest.cpp
psu_unittest_LDADD = $(top_builddir)/libsyscmds_common.la
