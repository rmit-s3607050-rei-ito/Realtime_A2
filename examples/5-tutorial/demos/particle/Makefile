
EXE = ogl2particle_linux
OBJS = draw.o ogl2particle.o shader.o

# release settings
#CFLAGS = -O2
#LFLAGS = -s

# debug settings
CFLAGS = -g
LFLAGS = -g

all: ${EXE}

${EXE}: ${OBJS}
	${LINK.cpp} ${LFLAGS} -o $@ ${OBJS} -lGLEW -lglut -lGL -lGLU

clean:
	-rm -f ${OBJS}

clobber: clean
	-rm -f ${EXE}

