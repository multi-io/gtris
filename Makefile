CXX = egcs
INCDIRS = -I/usr/lib/glib/include
LIBDIRS = -L/usr/X11R6/lib

LIBS = -lgdk -lgtk -lglib -lXext -lX11 -lm -lstdc++ # `gtk-config --libs`
OBJECTS = main.o gtkbrickviewer.o TetrisGameProcess.o types.o registry.o HighscoresManager.o options.o msgbox.o xpms.o

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
	@echo generating xpms.cc ...;                                        \
	(                                                                    \
	echo '/* xpms.cc : automatically generated source file containing';  \
	echo ' * all XPM pictures needed by gtris';                          \
	echo ' */';                                                          \
	for xpm in $^; do                                                    \
	    echo;                                                            \
	    echo;                                                            \
	    echo char\* `basename $$xpm .xpm`_xpm [] =;                      \
	    echo {;                                                          \
	    cat $$xpm | grep -Ev '/\*|\*/|\{|\}|static';                     \
	    echo }\;;                                                        \
	done                                                                 \
	) >xpms.cc

xpms.h :
	@echo generating xpms.h ...;                                         \
	(                                                                    \
	echo '/* xpms.h : automatically generated header file';              \
	echo ' * for xpms.cc';                                               \
	echo ' */';                                                          \
	echo;                                                                \
	for xpm in $(XPMS); do                                               \
	    echo extern char\* `basename $$xpm .xpm`_xpm []\;;               \
	done                                                                 \
	) >xpms.h


gtris: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIRS) $(LIBS) -o gtris

gtkbrickviewer.o main.o TetrisGameProcess.o types.o: types.h
HighscoresManager.o main.o registry.o: registry.h
main.o options.o TetrisGameProcess.o: TetrisGameProcess.h
main.o msgbox.o: msgbox.h
main.o options.o: options.h
gtkbrickviewer.o main.o: gtkbrickviewer.h
HighscoresManager.o main.o: HighscoresManager.h
main.o: xpms.h


.PHONY: clean
clean:
	rm -f $(OBJECTS) xpms.cc xpms.h gtris
