project(libkdegames)

add_subdirectory( carddecks ) 
add_subdirectory( highscore ) 
add_subdirectory( kgame ) 
add_subdirectory( pics ) 

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/highscore/ ${CMAKE_CURRENT_SOURCE_DIR}/kgame/ ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs/ )


########### next target ###############

set(libkhighscore_SRCS  
	highscore/kconfigrawbackend.cpp 
	highscore/kfilelock.cpp 
	highscore/khighscore.cpp 
	highscore/kscoredialog.cpp
	highscore/kexthighscore_item.cpp 
	highscore/kexthighscore_internal.cpp 
	highscore/kexthighscore_tab.cpp 
	highscore/kexthighscore_gui.cpp
	highscore/kexthighscore.cpp
)

set(libkgame_SRCS 
	kgame/kgame.cpp 
	kgame/kplayer.cpp 
	kgame/kgamenetwork.cpp 
	kgame/kgameproperty.cpp
	kgame/kgamemessage.cpp 
	kgame/kgameio.cpp 
	kgame/kgameprocess.cpp 
	kgame/kgamechat.cpp
	kgame/kgamepropertyhandler.cpp 
	kgame/kgameerror.cpp 
	kgame/kgamesequence.cpp
	kgame/kmessageio.cpp 
	kgame/kmessageserver.cpp 
	kgame/kmessageclient.cpp
	)

set(libkgamedialog_SRCS 
	kgame/dialogs/kgamedialog.cpp 
	kgame/dialogs/kgameconnectdialog.cpp 
	kgame/dialogs/kgameerrordialog.cpp 
	kgame/dialogs/kgamedebugdialog.cpp 
	kgame/dialogs/kgamedialogconfig.cpp
	)

set(kdegames_LIB_SRCS ${libkhighscore_SRCS} ${libkgame_SRCS} ${libkgamedialog_SRCS}
   kcarddialog.cpp 
   kstdgameaction.cpp 
   kgamemisc.cpp 
   kchatbase.cpp 
   kchat.cpp 
   kchatdialog.cpp 
   kgameprogress.cpp 
   kcanvasrootpixmap.cpp 
   kgamelcd.cpp )

kde4_automoc(${kdegames_LIB_SRCS})

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames  ${KDE4_KDE3SUPPORT_LIBS} kdnssd ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kdegames PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kdegames )


########### install files ###############

install_files(/include FILES
   libkdegames_export.h
   kgamemisc.h
   kcarddialog.h
   kstdgameaction.h
   kchatbase.h
   kchat.h
   kchatdialog.h
   kgameprogress.h
   kcanvasrootpixmap.h
   kgamelcd.h
   kgrid2d.h
)




#original Makefile.am contents follow:

#
#lib_LTLIBRARIES = libkdegames.la
#libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 3:0:2
#libkdegames_la_LIBADD  = highscore/libkhighscore.la kgame/libkgame.la kgame/dialogs/libkgamedialogs.la \
#		$(LIB_KSYCOCA) $(LIB_KDNSSD)
#
#libkdegames_la_SOURCES = kcarddialog.cpp kstdgameaction.cpp \
#		kgamemisc.cpp kchatbase.cpp kchat.cpp \
#		kchatdialog.cpp kgameprogress.cpp \
#        kcanvasrootpixmap.cpp kgamelcd.cpp
#include_HEADERS = kgamemisc.h kcarddialog.h kstdgameaction.h \
#		kchatbase.h kchat.h kchatdialog.h \
#        kgameprogress.h kcanvasrootpixmap.h kgamelcd.h kgrid2d.h
#
#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#SUBDIRS = carddecks highscore kgame pics
#
#messages:
#	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 
#
#include ../admin/Doxyfile.am
