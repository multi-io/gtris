#  $Id: devel.make,v 1.1.2.2 2000/01/30 04:22:29 olaf Exp $

REVISION:=$(shell echo $(filter release-%,$Name:  $) | sed 's/release-//g' | sed 's/-/./g' )

all: Makefile
	$(MAKE)

Makefile: Makefile.in configure
	rm -f config.cache 2>/dev/null || true
	CXXFLAGS='-g -O0' ./configure

configure: configure.in revision
	autoconf configure.in >configure
	chmod u+x configure

revision:
	echo $(REVISION) >revision

clean:
	( test -f Makefile && $(MAKE) clean ) || true
	rm -f configure config.* Makefile revision


tgz distrib: configure Makefile.in
	@DIRNAME=gtris-$(REVISION); \
	TGZNAME=$${DIRNAME}.tar.gz; \
	echo making $$TGZNAME ...; \
	mkdir $$DIRNAME; \
	rm -rf $$DIRNAME/*; \
	cp *.cc *.h *.xpm configure configure.in revision install-sh \
	   Makefile.in hscores.bin COPYING INSTALL $$DIRNAME/; \
	rm -f $$DIRNAME/xpms.* 2>/dev/null; \
	tar c $$DIRNAME/ | gzip >$$TGZNAME; \
	rm -rf $$DIRNAME/
