CC = g++
CFLAGS = -Wall
PROG = game

SRCS = 3Dgame.cpp
LIBS = glad.c -I /usr/local/include/GLFW -I /usr/include/irrklang -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lXi -ldl -pthread ikpMP3.so libIrrKlang.so

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) -O2 $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
