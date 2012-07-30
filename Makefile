# waveplawt linux makefile

CC=g++ -g
CFLAGS=-c -Wall
LIBS=-lGL -lGLU -lSDL
SOURCES=shader.cpp slider.cpp utils.cpp text.cpp lin_alg.cpp
OBJS=shader.o text.o utils.o slider.o lin_alg.o
OBJDIR=objs
SRCDIR=src
objects = $(addprefix $(OBJDIR)/, $(OBJS))
# on intel, the OpenCL headers lie in a weird place..
INCLUDE=-I/usr/lib64/OpenCL/global/include
EXECUTABLE=plot

all: waveplot

waveplot: $(objects)
	$(CC) $(LIBS) $(objects) src/waveplot.cpp $(INCLUDE) -o $(EXECUTABLE)

$(OBJDIR)/slider.o: src/slider.cpp
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

$(OBJDIR)/shader.o: src/shader.cpp
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

$(OBJDIR)/text.o: src/text.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/utils.o: src/utils.cpp
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/lin_alg.o: src/lin_alg.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLE) $(OBJDIR)/*.o
