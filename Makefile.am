
lib_LTLIBRARIES = libkdegames.la
libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 3:0:2
libkdegames_la_LIBADD  = highscore/libkhighscore.la kgame/libkgame.la kgame/dialogs/libkgamedialogs.la \
		$(LIB_KSYCOCA)

libkdegames_la_SOURCES = kcarddialog.cpp kstdgameaction.cpp \
		kgamemisc.cpp kchatbase.cpp kchat.cpp \
		kchatdialog.cpp kgameprogress.cpp \
        kcanvasrootpixmap.cpp kgamelcd.cpp
include_HEADERS = kgamemisc.h kcarddialog.h kstdgameaction.h \
		kchatbase.h kchat.h kchatdialog.h \
        kgameprogress.h kcanvasrootpixmap.h kgamelcd.h kgrid2d.h

INCLUDES = $(all_includes)
METASOURCES = AUTO

SUBDIRS = carddecks highscore kgame pics

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

include ../admin/Doxyfile.am
