
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

lib_LTLIBRARIES = libkdegames.la
libkdegames_la_SOURCES = kcarddialog.cpp kstdgameaction.cpp kgamemisc.cpp khighscore.cpp
libkdegames_la_LIBADD  = $(LIB_KSYCOCA)
libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:0

include_HEADERS = kcarddialog.h kstdgameaction.h kgamemisc.h khighscore.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

SUBDIRS = carddecks

messages:
	$(XGETTEXT) *.cpp -o $(podir)/libkdegames.pot 
