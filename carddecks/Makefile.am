DECKDIRS = cards-aisleriot cards-dondorf-whist-b cards-gdkcard-bonded cards-hard-a-port \
	cards-penguins cards-spaced cards-xskat-french cards1 decks

deckdir = $(kde_datadir)/carddecks

install-data-local:
	$(mkinstalldirs) $(deckdir)
	for i in $(DECKDIRS); do \
	if test ! -d $(DESTDIR)$(deckdir)/$$i; then \
		cp -r $(srcdir)/$$i $(DESTDIR)$(deckdir)/$$i ;\
		rm -rf $(DESTDIR)$(deckdir)/$$i/CVS ;\
	fi ;\
	done      
