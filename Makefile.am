
lib_LTLIBRARIES = libkdegames.la
libkdegames_la_SOURCES = kcarddialog.cpp
libkdegames_la_LIBADD   = $(LIB_KFILE)
libkdegames_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:0

include_HEADERS = kcarddialog.h

INCLUDES = $(all_includes)
METASOURCES = AUTO





messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdegames.pot 

