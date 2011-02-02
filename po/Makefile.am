top_builddir = ..

# -- Build

POFILES = $(wildcard *.po)
MOFILES = $(patsubst %.po,%.mo,$(POFILES)) 

CLEANFILES = $(MOFILES)

localedir=$(datadir)/locale

MSGFMT = msgfmt

all: $(MOFILES)

%.mo: %.po
	$(MSGFMT) -o t-$@ $< && mv t-$@ $@

install-data-hook: $(MOFILES)
	for cat in "" $(MOFILES); do \
	  if test -z "$$cat"; then continue; fi; \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.mo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkdir_p) $(DESTDIR)$$dir; \
	  if test -r $$cat; then realcat=$$cat; else realcat=$(srcdir)/$$cat; fi; \
	  $(INSTALL) -m 644 $$realcat $(DESTDIR)$$dir/$(PACKAGE).mo; \
	 done

# -- Manage translations

SOURCES = $(shell cd $(top_builddir); find . -name '*.[hc]')

$(PACKAGE).pot:
	cd $(top_builddir); xgettext -k_ -kN_ -d $(PACKAGE) \
		--from-code=UTF-8 \
		--foreign-user \
		--package-name="$(PACKAGE)" --package-version="$(VERSION)" \
		--msgid-bugs-address="$(PACKAGE_BUGREPORT)" \
		-o $(abs_srcdir)/$@ $(SOURCES)

# .tx

push-source: $(PACKAGE).pot
	tx push -s

push-translations:
	if [ "$fORCE" = yes ]; then tx push -t; fi

pull-translations:
	tx pull -a

.PHONY: $(PACKAGE).pot
