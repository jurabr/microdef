AC_PREREQ(2.60)
AC_INIT([microdef], [0.1], [jirka@penguin.cz])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9.6 -Wall -Werror dist-bzip2])

AC_PROG_CC
# Compiling sources with per-target flags requires AM_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_GTK_2_0([2.0.0],,AC_MSG_ERROR([Gtk+ 2.0.0 or higher required.]))

#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
])
AC_OUTPUT
