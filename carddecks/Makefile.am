DECKDIRS = cards-aisleriot cards-dondorf-whist-b cards-gdkcard-bonded cards-hard-a-port \
	cards-penguins cards-spaced cards-xskat-french cards-default decks cards-konqi-modern

deckdir = $(kde_datadir)/carddecks

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(deckdir)
	@for i in $(DECKDIRS); do \
	if test ! -d $(DESTDIR)$(deckdir)/$$i; then \
		echo "installing $$i" ;\
		mkdir $(DESTDIR)$(deckdir)/$$i ;\
		files=`cd $(srcdir)/$$i && ls -1d *` ;\
		for f in $$files; do \
			if test -f $(srcdir)/$$i/$$f; then \
				$(INSTALL_DATA) $(srcdir)/$$i/$$f $(DESTDIR)$(deckdir)/$$i/$$f ;\
			fi \
		done \
	fi ;\
	done      
