DEBUG = -g
OPTIMISE = -O2

CFLAGS = `sdl2-config --cflags` $(DEBUG) $(OPTIMISE) -std=c99 -Wall
LDFLAGS = `sdl2-config --libs` -lGL -lGLU -lglut -lSDL2 -lm

HEADERS = sdl-base.h shaders.h
OBJECTS = tute-shaders.o sdl-base.o shaders.o
EXE = tute-shaders

all: $(EXE)

$(EXE): $(OBJECTS) $(HEADERS)
	gcc -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJECTS)
