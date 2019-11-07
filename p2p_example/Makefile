CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o p2p_example $(CFLAGS) $(CFILES) $(LIBS)

clean:
	rm p2p_example
