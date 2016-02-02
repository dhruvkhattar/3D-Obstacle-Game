CC = g++
CFLAGS = -Wall
PROG = game

SRCS = 3Dgame.cpp
LIBS = glad.c -I /usr/local/include/GLFW -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr  -lXi -ldl 

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) -O2 $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
