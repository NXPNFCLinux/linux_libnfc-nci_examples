CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o ndef-emulation_example $(CFLAGS) $(CFILES) $(LIBS)

clean:
	rm ndef-emulation_example
