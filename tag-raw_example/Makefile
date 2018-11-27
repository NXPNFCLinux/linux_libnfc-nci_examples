CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o tag-raw_example $(CFLAGS) $(CFILES) $(LIBS)

clean:
	rm tag-raw_example
