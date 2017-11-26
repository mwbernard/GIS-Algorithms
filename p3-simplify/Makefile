PLATFORM = $(shell uname)


## Compilation flags
##comment out one or the other 
##debugging 
#CFLAGS = -g 
##release
CFLAGS = -O3 -DNDEBUG
LDFLAGS=

CFLAGS+= -Wall

ifeq ($(PLATFORM),Darwin)
## Mac OS X
CFLAGS += -m64 -isystem/usr/local/include
LDFLAGS+= -m64 -lc -framework AGL -framework OpenGL -framework GLUT -framework Foundation
else
## Linux
CFLAGS += -m64
INCLUDEPATH  = -I/usr/include/GL/ 
LIBPATH = -L/usr/lib64 -L/usr/X11R6/lib
LDFLAGS+=  -lGL -lglut -lrt -lGLU -lX11 -lm  -lXmu -lXext -lXi
endif


CC = gcc -O3 -Wall $(INCLUDEPATH)


PROGS = simplify

default: $(PROGS)

simplify: simplify.o grid.o rtimer.o
	$(CC) -o $@ simplify.o grid.o rtimer.o $(LDFLAGS)

simplify.o: simplify.c  grid.h rtimer.h
	$(CC) -c $(INCLUDEPATH)  simplify.c  -o $@

grid.o: grid.c grid.h 
	$(CC) -c $(INCLUDEPATH)  grid.c -o $@

rtimer.o: rtimer.h rtimer.c
	$(CC) -c $(INCLUDEPATH) rtimer.c -o $@

clean::	
	rm *.o
	rm simplify


