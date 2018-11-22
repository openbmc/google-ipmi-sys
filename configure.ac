# Initialization
AC_PREREQ([2.69])
AC_INIT([sys-ipmi], [0.1], [https://www.github.com/google-ipmi-sys/issues])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_17([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

# Checks for libraries.
PKG_CHECK_MODULES(
    [SDBUSPLUS],
    [sdbusplus],
    [],
    [AC_MSG_ERROR([The openbmc/sdbusplus package is required])]
)
PKG_CHECK_MODULES(
    [PHOSPHOR_LOGGING],
    [phosphor-logging],
    [],
    [AC_MSG_ERROR([The openbmc/phosphor-logging package is required])]
)
AC_CHECK_HEADER(
    experimental/filesystem,
    [],
    [AC_MSG_ERROR([Could not find experimental/filesystem...libstdc++fs developement package required])]
)
AC_CHECK_HEADER(
    [host-ipmid],
    [AC_MSG_ERROR(["phosphor-host-ipmid required and not found."])]
)

# Checks for library functions.
LT_INIT # Required for systemd linking

PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemdsystemunitdir],
     [AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files])],,
     [with_systemdsystemunitdir=auto])
AS_IF([test "x$with_systemdsystemunitdir" = "xyes" -o "x$with_systemdsystemunitdir" = "xauto"], [
     def_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)

     AS_IF([test "x$def_systemdsystemunitdir" = "x"],
   [AS_IF([test "x$with_systemdsystemunitdir" = "xyes"],
    [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])])
    with_systemdsystemunitdir=no],
   [with_systemdsystemunitdir="$def_systemdsystemunitdir"])])
AS_IF([test "x$with_systemdsystemunitdir" != "xno"],
      [AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

# Create configured output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
