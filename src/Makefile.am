INCLUDES = @EVAS_CFLAGS@ @EDJE_CFLAGS@ @ECORE_EVAS_CFLAGS@
AM_CFLAGS=-Wall -Werror -DDATADIR='"$(datadir)"'

bin_PROGRAMS = etimetool

etimetool_SOURCES = etimetool.c
etimetool_LDADD = @EVAS_LIBS@ @EDJE_LIBS@ @ECORE_EVAS_LIBS@
etimetool_LDFLAGS = -Wl,--as-needed