kde4_header()
project(libkdegames)

add_subdirectory( carddecks ) 
add_subdirectory( highscore ) 
add_subdirectory( kgame ) 
add_subdirectory( pics ) 

include_directories( 
	${CMAKE_SOURCE_DIR}/libkdegames/ 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/ 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/ 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/ 
	${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(libkhighscore_SRCS  
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kconfigrawbackend.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kfilelock.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/khighscore.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kscoredialog.cpp
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kexthighscore_item.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kexthighscore_internal.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kexthighscore_tab.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kexthighscore_gui.cpp
	${CMAKE_SOURCE_DIR}/libkdegames/highscore/kexthighscore.cpp
)

set(libkgame_SRCS 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgame.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kplayer.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgamenetwork.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgameproperty.cpp
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgamemessage.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgameio.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgameprocess.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgamechat.cpp
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgamepropertyhandler.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgameerror.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kgamesequence.cpp
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kmessageio.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kmessageserver.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/kmessageclient.cpp
	)

set(libkgamedialog_SRCS 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/kgamedialog.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/kgameconnectdialog.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/kgameerrordialog.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/kgamedebugdialog.cpp 
	${CMAKE_SOURCE_DIR}/libkdegames/kgame/dialogs/kgamedialogconfig.cpp
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

target_link_libraries(kdegames  ${KDE4_KDE3SUPPORT_LIBRARIES} kdnssd ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kdegames PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kdegames )


########### install files ###############

install_files( /include FILES kgamemisc.h kcarddialog.h kstdgameaction.h  		kchatbase.h kchat.h kchatdialog.h          kgameprogress.h kcanvasrootpixmap.h kgamelcd.h kgrid2d.h )

kde4_create_doxygen_docs(  )

kde4_footer()



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
