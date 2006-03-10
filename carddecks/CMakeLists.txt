kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/carddecks FILES  )

kde4_footer()



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
