



########### next target ###############


########### install files ###############

install(FILES
   khighscore.h
   kscoredialog.h
   kexthighscore_item.h
   kexthighscore.h
DESTINATION ${INCLUDE_INSTALL_DIR})




#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libkhighscore.la
#
#INCLUDES = $(all_includes)
#
#libkhighscore_la_SOURCES =  kconfigrawbackend.cpp \
#	                    kfilelock.cpp khighscore.cpp kscoredialog.cpp \
#                            kexthighscore_item.cpp kexthighscore_internal.cpp \
#                            kexthighscore_tab.cpp kexthighscore_gui.cpp \
#                            kexthighscore.cpp
#
#include_HEADERS = khighscore.h kscoredialog.h \
#                  kexthighscore_item.h kexthighscore.h
#
#noinst_HEADERS = kconfigrawbackend.h \
#                 kfilelock.h kexthighscore_internal.h kexthighscore_tab.h \
#                 kexthighscore_gui.h
#
#METASOURCES = kconfigrawbackend.moc khighscore.moc kscoredialog.moc \
#              kexthighscore_tab.moc kexthighscore_gui.moc
