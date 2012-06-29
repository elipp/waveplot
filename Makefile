# waveplawt linux makefile

CC=g++ -g
CFLAGS=-c -Wall
LIBS=-lGL -lGLU -lSDL
# on intel, the OpenCL headers lie in a weird place..
INCLUDE=-I/usr/lib64/OpenCL/global/include
EXECUTABLE=plot

all: waveplot

waveplot: shader.o text.o utils.o 
	$(CC) $(LIBS) shader.o text.o utils.o waveplot.cpp $(INCLUDE) -o $(EXECUTABLE)

shader.o: shader.cpp
	$(CC) $(CFLAGS) $(LIBS) shader.cpp

text.o: text.cpp
	$(CC) $(CFLAGS) text.cpp

utils.o: utils.cpp
	$(CC) $(CFLAGS) utils.cpp

clean:
	rm -rf $(EXECUTABLE) *.o
