kde4_header()

add_subdirectory( carddecks ) 
add_subdirectory( highscore ) 
add_subdirectory( kgame ) 
add_subdirectory( pics ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdegames_LIB_SRCS 
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

target_link_libraries(kdegames  ${QT_AND_KDECORE_LIBS} khighscore kgame kgamedialogs kio kdnssd )

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
