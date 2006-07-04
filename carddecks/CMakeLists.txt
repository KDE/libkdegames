
# will be used in subdirs' CMakeLists.txt
set( carddecks_base_dir ${CMAKE_CURRENT_SOURCE_DIR} )

# when adding new deck subdirectory it should be sufficient to just
# copy CMakeLists.txt from any existing deckdir to the new one.
add_subdirectory( cards-aisleriot )
add_subdirectory( cards-dondorf-whist-b )
add_subdirectory( cards-gdkcard-bonded )
add_subdirectory( cards-hard-a-port )
add_subdirectory( cards-penguins )
add_subdirectory( cards-spaced )
add_subdirectory( cards-xskat-french )
add_subdirectory( cards-default)
add_subdirectory( decks )
add_subdirectory( cards-konqi-modern )
add_subdirectory( cards-warwick)
add_subdirectory( cards-xskat-german)

########### install files ###############





#original Makefile.am contents follow:

#DECKDIRS = cards-aisleriot cards-dondorf-whist-b cards-gdkcard-bonded cards-hard-a-port \
#	cards-penguins cards-spaced cards-xskat-french cards-default decks cards-konqi-modern cards-warwick cards-xskat-german
#
#deckdir = $(kde_datadir)/carddecks
#
#uninstall-local:
#	rm -rf $(DESTDIR)$(deckdir)
#
#install-data-local:
#	@$(mkinstalldirs) $(DESTDIR)$(deckdir)
#	@for i in $(DECKDIRS); do \
#		if test -d $(DESTDIR)$(deckdir)/$$i; then \
#			echo "refreshing and removing $$i" ;\
#			rm -rf $(DESTDIR)$(deckdir)/$$i;\
#		fi ;\
#		echo "installing $$i" ;\
#		mkdir $(DESTDIR)$(deckdir)/$$i ;\
#		files=`cd $(srcdir)/$$i && ls -1d *` ;\
#		for f in $$files; do \
#			if test -f $(srcdir)/$$i/$$f; then \
#				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(deckdir)/$$i/$$f ;\
#			fi \
#		done \
#	done      
