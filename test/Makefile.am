AM_CPPFLAGS = \
	-I$(top_srcdir)/ \
	$(GTEST_CFLAGS) \
	$(GMOCK_CFLAGS)
AM_LDFLAGS = \
	$(GTEST_LIBS) \
	$(GMOCK_LIBS) \
	-lgmock_main \
	$(OESDK_TESTCASE_FLAGS)

check_PROGRAMS =
TESTS = $(check_PROGRAMS)

check_PROGRAMS += eth_unittest
eth_unittest_SOURCES = eth_unittest.cpp
eth_unittest_LDADD = $(top_builddir)/eth.o
