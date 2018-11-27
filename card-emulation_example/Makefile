CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o card-emulation_example $(CFLAGS) $(CFILES) $(CFILES_PN5XX) $(LIBS)

clean:
	rm card-emulation_example
