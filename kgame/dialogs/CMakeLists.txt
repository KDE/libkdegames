kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/kgame ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( /include/kgame FILES kgamedialog.h kgameconnectdialog.h kgameerrordialog.h kgamedebugdialog.h kgamedialogconfig.h )

kde4_footer()



#original Makefile.am contents follow:

#
#noinst_LTLIBRARIES = libkgamedialogs.la
#
## compile-order doesn't matter here but maybe we will split these section soon
#
#
#libkgamedialogs_la_SOURCES = kgamedialog.cpp kgameconnectdialog.cpp kgameerrordialog.cpp kgamedebugdialog.cpp kgamedialogconfig.cpp
#
#libkgamedialogsincludedir=$(includedir)/kgame
#libkgamedialogsinclude_HEADERS = kgamedialog.h kgameconnectdialog.h kgameerrordialog.h kgamedebugdialog.h kgamedialogconfig.h
#
#INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/kgame $(all_includes)
#METASOURCES = AUTO
#
#messages:
##	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 
#
