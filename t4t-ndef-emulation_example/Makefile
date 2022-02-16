CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o t4t-ndef-emulation_example $(CFLAGS) $(CFILES) $(LIBS)

clean:
	rm t4t-ndef-emulation_example
