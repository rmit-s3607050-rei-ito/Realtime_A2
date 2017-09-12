DEBUG = -g
OPTIMISE = -O2

CFLAGS = `sdl2-config --cflags` $(DEBUG) $(OPTIMISE) -std=c++14 -Wall
LDFLAGS = `sdl2-config --libs` -lGL -lGLU -lglut -lm

OBJECTS = sinewave3D-glm.cpp
EXE = sinewave

all: $(EXE)

$(EXE): $(OBJECTS)
	g++ -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(EXE)
