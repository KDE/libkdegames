
CPPFLAGS = -DSRCDIR="\"$(shell (cd $(srcdir) && pwd))\""

noinst_LTLIBRARIES = libkgame.la

# compile-order doesn't matter here but maybe we will split these section soon

KGAME = kgame.cpp kplayer.cpp kgamenetwork.cpp kgameproperty.cpp kgamemessage.cpp kgameio.cpp kgameprocess.cpp kgamechat.cpp
KGAME_H = kgame.h kplayer.h kgamenetwork.h kgameproperty.h kgamemessage.h kgameio.h kgameprocess.h kgamepropertyarray.h kgamepropertylist.h kgamechat.h

KGAME_DIALOGS = 
#kgamedialog.cpp kgamechat.cpp kgameconnectdialog.cpp kgameerrordialog.cpp kgamedebugdialog.cpp
KGAME_DIALOGS_H = 
#kgamedialog.h kgamechat.h kgameconnectdialog.h kgameerrordialog.h kgamedebugdialog.h

KMESSAGE = kmessageio.cpp kmessageserver.cpp kmessageclient.cpp
KMESSAGE_H = kmessageio.h kmessageserver.h kmessageclient.h

libkgame_la_SOURCES = $(KMESSAGE) $(KGAME) $(KGAME_DIALOGS)

include_HEADERS = $(KMESSAGE_H) $(KGAME_H) $(KGAME_DIALOGS_H)

INCLUDES = $(all_includes) -I$(top_srcdir)/libkdegames
METASOURCES = AUTO

SUBDIRS = dialogs

messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

