
CC = gcc
OBJECTS = ../widget/loadtiff.o tiff2tga.o
LIBS = -lm
LDFLAGS =
CFLAGS = -Wall -pipe -O3 -fno-strength-reduce -I../widget -I../
#CFLAGS = -Wall -pipe -g

.PHONY: clean

all: tifftga

tifftga: $(OBJECTS)
	$(CC) $(LDFLAGS) -o tiff2tga $(OBJECTS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<
