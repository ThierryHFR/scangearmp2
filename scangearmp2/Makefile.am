## Process this file with automake to produce Makefile.in

SUBDIRS = po src etc include

EXTRA_DIST = \
	autogen.sh \
	scangearmp2.glade

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir); \
	for glade in $(srcdir)/*.glade; do \
	  if test -f $$glade; then \
	    $(INSTALL_DATA) $$glade $(DESTDIR)$(pkgdatadir); \
	  fi \
	done
	
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi

dist-hook:
	mkdir $(distdir); \
	for glade in *.glade; do \
	  if test -f $$glade; then \
	    cp -p $$glade $(distdir); \
	  fi \
	done
	
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

