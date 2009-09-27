INCLUDES = @EFL_CFLAGS@
AM_CFLAGS=-Wall -Werror -DDATADIR='"$(datadir)"'

bin_PROGRAMS = etimetool

etimetool_SOURCES = etimetool.c
etimetool_LDADD = @EFL_LIBS@
etimetool_LDFLAGS = -Wl,--as-needed
