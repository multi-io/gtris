#  $Id: Makefile,v 1.5.4.2 1999/10/17 09:30:10 olaf Exp $

#  GTris
#  $Name:  $
#  Copyright (C) 1999  Olaf Klischat
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#where to put things into on 'make install'
PREFIX=/usr/X11R6
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/X11/lib/gtris    #not used (yet)

CXX = egcs
INCDIRS = -I/usr/lib/glib/include
LIBDIRS = -L/usr/X11R6/lib

LIBS = -lgdk -lgtk -lglib -lXext -lX11 -lm -lstdc++ # `gtk-config --libs`
OBJECTS = \
	main.o                \
	gtkbrickviewer.o      \
	TetrisGameProcess.o   \
	utils.o               \
	registry.o            \
	HighscoresManager.o   \
	options.o             \
	msgbox.o              \
	xpms.o

default: release

debug:
	$(MAKE) gtris CXXFLAGS=-g

release:
	$(MAKE) gtris CXXFLAGS=-O2
	strip gtris

%.o : %.cc
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) $< -o $@

XPMS = new.xpm run.xpm stop.xpm pause.xpm highscores.xpm options.xpm

xpms.cc : $(XPMS)
	@echo generating xpms.cc ...; \
	( \
	echo '/* xpms.cc : automatically generated source file containing'; \
	echo ' * all XPM pictures needed by gtris'; \
	echo ' */'; \
	for xpm in $^; do \
	    echo; \
	    echo; \
	    echo char\* `basename $$xpm .xpm`_xpm [] =; \
	    echo {; \
	    cat $$xpm | grep -Ev '/\*|\*/|\{|\}|static'; \
	    echo }\;; \
	done \
	) >xpms.cc

xpms.h :
	@echo generating xpms.h ...; \
	( \
	echo '/* xpms.h : automatically generated header file'; \
	echo ' * for xpms.cc'; \
	echo ' */'; \
	echo; \
	for xpm in $(XPMS); do \
	    echo extern char\* `basename $$xpm .xpm`_xpm []\;; \
	done \
	) >xpms.h


gtris: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIRS) $(LIBS) -o gtris

HighscoresManager.o: HighscoresManager.h registry.h
TetrisGameProcess.o: TetrisGameProcess.h gtkbrickviewer.h utils.h
gtkbrickviewer.o: gtkbrickviewer.h utils.h
main.o: gtkbrickviewer.h utils.h TetrisGameProcess.h HighscoresManager.h
main.o: options.h msgbox.h registry.h xpms.h
msgbox.o: msgbox.h
options.o: options.h TetrisGameProcess.h gtkbrickviewer.h utils.h
registry.o: registry.h
utils.o: utils.h


ready_for_install:
	@if [ ! -x gtris ]; then \
	echo ERROR: executable \'gtris\' not found. Use \'make\' first. >&2; \
	exit 1; \
	fi;

install: ready_for_install
	install gtris $(BINDIR)/gtris;

REVISION:=$(shell echo $(filter release-%,$Name:  $) | sed 's/release-//g' | sed 's/-/./g' )

tgz distrib:
	@DIRNAME=gtris-$(REVISION); \
	TGZNAME=$${DIRNAME}.tar.gz; \
	echo making $$TGZNAME ...; \
	mkdir $$DIRNAME; \
	rm -Rf $$DIRNAME/*; \
	cp *.cc *.h $(XPMS) Makefile hscores.bin COPYING INSTALL $$DIRNAME/; \
	rm -f $$DIRNAME/xpms.* 2>/dev/null; \
	tar c $$DIRNAME/ | gzip >$$TGZNAME; \
	rm -Rf $$DIRNAME/

.PHONY: clean
clean:
	rm -f $(OBJECTS) xpms.cc xpms.h gtris


