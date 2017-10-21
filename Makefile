#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread 
OBJS = 

all: pgm1

pgm1:
	$(CC) -o webserver webserver.c $(CFLAGS) $(LIBS)

clean:
	rm -f webserver
