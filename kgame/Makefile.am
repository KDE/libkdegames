
noinst_LTLIBRARIES = libkgame.la

# compile-order doesn't matter here but maybe we will split these section soon

KGAME = kgame.cpp kplayer.cpp kgamenetwork.cpp kgameproperty.cpp \
		kgamemessage.cpp kgameio.cpp kgameprocess.cpp kgamechat.cpp \
		kgamepropertyhandler.cpp kgameerror.cpp kgamesequence.cpp
KGAME_H = kgame.h kplayer.h kgamenetwork.h kgameproperty.h kgamemessage.h \
		kgameio.h kgameprocess.h kgamepropertyarray.h \
		kgamepropertylist.h kgamechat.h kgamepropertyhandler.h \
		kgameerror.h kgamesequence.h kgameversion.h

KMESSAGE = kmessageio.cpp kmessageserver.cpp kmessageclient.cpp
KMESSAGE_H = kmessageio.h kmessageserver.h kmessageclient.h

libkgameincludedir=$(includedir)/kgame
libkgame_la_SOURCES = $(KMESSAGE) $(KGAME)

libkgameinclude_HEADERS = $(KMESSAGE_H) $(KGAME_H)

INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)
METASOURCES = AUTO

SUBDIRS = . dialogs

messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

