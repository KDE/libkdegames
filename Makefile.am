
AM_CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

lib_LTLIBRARIES = libkdegames.la
libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:1
libkdegames_la_LIBADD  = $(top_builddir)/libkdegames/kgame/libkgame.la \
		$(top_builddir)/libkdegames/kgame/dialogs/libkgamedialogs.la \
		$(LIB_KSYCOCA)

libkdegames_la_SOURCES = kcarddialog.cpp kstdgameaction.cpp \
		kgamemisc.cpp khighscore.cpp kchatbase.cpp kchat.cpp \
		kchatdialog.cpp kscoredialog.cpp kgameprogress.cpp \
        kcanvasrootpixmap.cpp
include_HEADERS = kgamemisc.h kcarddialog.h kstdgameaction.h \
		khighscore.h kchatbase.h kchat.h kchatdialog.h kscoredialog.h \
        kgameprogress.h kcanvasrootpixmap.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

SUBDIRS = carddecks kgame pics

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 
