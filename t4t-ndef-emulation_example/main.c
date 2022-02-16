#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "linux_nfc_api.h"

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned char fileID[] = {0xE1, 0x04};


int main(int argc, char ** argv) {
    int res;
    unsigned char NDEFMsg[100];
    int NDEFMsgLen = 0;
    unsigned char NDEFMsg_read[100];
    int NDEFMsgLen_read = sizeof(NDEFMsg_read);

    nfcManager_doInitialize();

    /* Create NDEF message */
    res = ndef_createText("en", "Hello world !", NDEFMsg, sizeof(NDEFMsg));
    if(res <= 0x00)
    {
        printf("Failed to build TEXT NDEF Message\n");
        exit(0);
    }
    else
    {
        NDEFMsgLen = res;
    }

    /* Write NDEF message into T4T NDEF emulation memory for further shraing over NFC interface */
    res = doWriteT4tData(fileID, NDEFMsg, NDEFMsgLen);
    if(res)
    {
        printf("Failed to write NDEF Message to T4T NDEF emulation\n");
        exit(0);
    }
    else
    {
        printf("NDEF Message written to T4T NDEF emulation memory\n");
    }

    /* Read NDEF message from T4T NDEF emulation memory */
    res = doReadT4tData(fileID, NDEFMsg_read, &NDEFMsgLen_read);
    if(res)
    {
        printf("Failed to read NDEF Message to T4T NDEF emulation\n");
        exit(0);
    }
    else
    {
        printf("NDEFMsgLen = %d\n", NDEFMsgLen);
        printf("NDEFMsgLen_read = %d\n", NDEFMsgLen_read);
        printf("strncmp = %d\n", strncmp((const char *) NDEFMsg, (const char *) NDEFMsg_read, NDEFMsgLen));

        if((NDEFMsgLen != NDEFMsgLen_read) || strncmp((const char *) NDEFMsg, (const char *) NDEFMsg_read, NDEFMsgLen))
        {
            printf("NDEF Message verification failed\n");
            exit(0);
        }
        else
        {
            printf("NDEF Message verification succeed\n");
        }
    }

    printf("Now starting discovery loop to expose T4T NDEF emulation over NFC interface \n");

    nfcManager_enableDiscovery(0x00, 0, 1, 0);

    printf("\n-------------\nWaiting for reader...\n\n");

    while(1);
}
