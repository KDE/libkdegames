
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

lib_LTLIBRARIES = libkdegames.la
libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:1
libkdegames_la_LIBADD  = $(top_builddir)/libkdegames/kgame/libkgame.la $(LIB_KSYCOCA)

picsdir = $(kde_datadir)/kdegames/pics
pics_DATA = star.png

libkdegames_la_SOURCES = kcarddialog.cpp kstdgameaction.cpp \
		kgamemisc.cpp khighscore.cpp kchatbase.cpp kchat.cpp
include_HEADERS = kgamemisc.h kcarddialog.h kstdgameaction.h \
		khighscore.h kchatbase.h kchat.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

SUBDIRS = carddecks

messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

