CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o tag-read_example $(CFLAGS) $(CFILES) $(LIBS)

clean:
	rm tag-read_example
