THEMES=$(wildcard *.edc)
COMPILED_THEMES=$(patsubst %.edc,%.edj,$(THEMES))

#%.edj: %.edc
#  edje_cc $< $@

themedir = $(pkgdatadir)/themes/
theme_DATA = $(COMPILED_THEMES)

