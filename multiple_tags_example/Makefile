CC=gcc

CFILES=main.c

INCLUDES= -I/usr/include

LIBS= -lnfc_nci_linux -lpthread

CFLAGS=-Wall $(INCLUDES)

all:
	$(CC) -o multiple-tags_example $(CFLAGS) $(CFILES) $(CFILES_PN5XX) $(LIBS)

clean:
	rm multiple-tags_example
