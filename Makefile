C++ = egcs
C++OPTS = -g
INCDIRS = -I/usr/lib/glib/include
LIBDIRS = -L/usr/X11R6/lib
LIBS = -lgdk -lgtk -lglib -lXext -lX11 -lm -lstdc++ # `gtk-config --libs`
OBJECTS = main.o gtkbrickviewer.o TetrisGameProcess.o types.o registry.o HighscoresManager.o options.o msgbox.o


gtris: ${OBJECTS}
	${C++} ${OBJECTS} ${LIBDIRS} ${LIBS} -o gtris

main.o: main.cc gtkbrickviewer.h TetrisGameProcess.h types.h registry.h HighscoresManager.h options.h
	${C++} -c ${C++OPTS} ${INCDIRS} main.cc -o main.o

gtkbrickviewer.o: gtkbrickviewer.cc gtkbrickviewer.h types.h
	${C++} -c ${C++OPTS} ${INCDIRS} gtkbrickviewer.cc -o gtkbrickviewer.o

TetrisGameProcess.o: TetrisGameProcess.cc TetrisGameProcess.h gtkbrickviewer.h types.h
	${C++} -c ${C++OPTS} ${INCDIRS} TetrisGameProcess.cc -o TetrisGameProcess.o

types.o: types.cc types.h
	${C++} -c ${C++OPTS} ${INCDIRS} types.cc -o types.o

registry.o: registry.cc registry.h
	${C++} -c ${C++OPTS} ${INCDIRS} registry.cc -o registry.o

HighscoresManager.o: HighscoresManager.cc HighscoresManager.h registry.h
	${C++} -c ${C++OPTS} ${INCDIRS} HighscoresManager.cc -o HighscoresManager.o

options.o: options.cc options.h TetrisGameProcess.h
	${C++} -c ${C++OPTS} ${INCDIRS} options.cc -o options.o

msgbox.o: msgbox.cc msgbox.h
	${C++} -c ${C++OPTS} ${INCDIRS} msgbox.cc -o msgbox.o


.PHONY: clean
clean:
	rm -f ${OBJECTS} gtris
