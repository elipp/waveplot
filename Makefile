# waveplawt linux makefile

CC=g++ -g
CFLAGS=-c -Wall
LIBS=-lGL -lGLU -lSDL
EXECUTABLE=plot

all: waveplot

waveplot: shader.o text.o utils.o 
	$(CC) $(LIBS) shader.o text.o utils.o waveplot.cpp -o $(EXECUTABLE)

shader.o: shader.cpp
	$(CC) $(CFLAGS) $(LIBS) shader.cpp

text.o: text.cpp
	$(CC) $(CFLAGS) text.cpp

utils.o: utils.cpp
	$(CC) $(CFLAGS) utils.cpp

clean:
	rm -rf $(EXECUTABLE) *.o
