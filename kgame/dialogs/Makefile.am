
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

noinst_LTLIBRARIES = libkgamedialogs.la

# compile-order doesn't matter here but maybe we will split these section soon


libkgamedialogs_la_SOURCES = kgamedialog.cpp kgameconnectdialog.cpp kgameerrordialog.cpp kgamedebugdialog.cpp kgamedialogconfig.cpp
include_HEADERS = kgamedialog.h kgameconnectdialog.h kgameerrordialog.h kgamedebugdialog.h kgamedialogconfig.h

INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/kgame $(all_includes)
METASOURCES = AUTO

SUBDIRS =

messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

