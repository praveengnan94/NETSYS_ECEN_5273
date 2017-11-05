#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread -lssl -lcrypto
OBJS = 

all: pgm1 

pgm1:
	$(CC) -o dfc dfc.c $(CFLAGS) $(LIBS)

clean:
	rm -f dfc
