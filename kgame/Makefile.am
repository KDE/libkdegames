
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

noinst_LTLIBRARIES = libkgame.la

KGAME=kgame.cpp kplayer.cpp kgameclient.cpp kgamenetwork.cpp kgameproperty.cpp kgamemessage.cpp kgameio.cpp kgameprocess.cpp kmessageio.cpp kmessageserver.cpp kmessageclient.cpp
KGAME_H=kgame.h kplayer.h kgameclient.h kgamenetwork.h kgameproperty.h kgamemessage.h kgameio.h kgameprocess.h kmessageio.h kmessageserver.h kmessageclient.h
KGAME_DIALOGS=kgamedialog.cpp kgamechat.cpp kgameconnectdialog.cpp kgameerrordialog.cpp 
KGAME_DIALOGS_H=kgamedialog.h kgamechat.h kgameconnectdialog.h kgameerrordialog.h

libkgame_la_SOURCES = $(KGAME) $(KGAME_DIALOGS)

include_HEADERS = $(KGAME_H) $(KGAME_DIALOGS_H)

INCLUDES = $(all_includes) -I$(top_srcdir)/libkdegames
METASOURCES = AUTO

SUBDIRS =

messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

