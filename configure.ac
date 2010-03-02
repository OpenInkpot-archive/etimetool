AC_PREREQ(2.57)
AC_INIT(etimetool, 0.2, avn@daemon.hole.ru)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(EFL,[ecore evas edje ecore-evas])
PKG_CHECK_MODULES(EOI,[libeoi])

AC_OUTPUT(Makefile src/Makefile po/Makefile themes/Makefile)
