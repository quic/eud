AC_INIT([libeud],[0.1],[eud-dev@quicinc.com],[libeud],[http://github.qualcomm.com/CorePnI/eud])

AC_CANONICAL_HOST

# Check build environment
case "${host_os}" in
    linux*)
        linux_env=yes
        ;;
    cygwin*|mingw*)
        windows_env=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac


# libusb-1.0 dependency check
PKG_CHECK_MODULES([libusb], [libusb-1.0],
	[HAVE_LIBUSB=yes], [HAVE_LIBUSB=no])

AS_IF([test "x$HAVE_LIBUSB" = "xyes"],
	[libusb_msg="yes"], [libusb_msg="no (missing: libusb-1.0)"])


# Store the auxiliary build tools (e.g., install-sh, config.sub, config.guess)
# in this dir (build-aux)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])

# Init automake, and specify this program use relaxed structures.
# i.e. this program doesn't follow the gnu coding standards, and doesn't have
# ChangeLog, COPYING, AUTHORS, INSTALL, README etc. files.
AM_INIT_AUTOMAKE([-Wall foreign])

# Check for C compiler
AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX

LT_INIT

AC_ARG_ENABLE([dll],
  AS_HELP_STRING([--enable-dll], [Build EUD driver dll for Windows]),
  [build_dll=$enableval], [build_dll=no])

AM_CONDITIONAL([BUILD_DLL],[test "x$build_dll" = "xyes"])

AM_CONDITIONAL([LINUX_ENV],  [test "x$linux_env" = "xyes"])
AM_CONDITIONAL([WINDOWS_ENV],[test "x$windows_env" = "xyes"])

AM_CONDITIONAL([SUBPROJECT_BUILD],
	[test "x$enable_subproject_build" = "xyes"])


# Create Makefile
AC_CONFIG_FILES([Makefile])


# Generate the output
AC_OUTPUT
