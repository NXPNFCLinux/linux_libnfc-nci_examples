#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "linux_nfc_api.h"

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t PeerDevicePresence_mutex;

static nfcSnepServerCallback_t g_SnepServerCB;
static nfcSnepClientCallback_t g_SnepClientCB;
static int isPeerDevicePresent = 0;

void onMessageReceived(unsigned char *message, unsigned int length)
{
    printf("NDEF Message Received\n");
}

void onSnepClientReady()
{
    printf("Peer device arrived\n");

    pthread_mutex_lock (&PeerDevicePresence_mutex);
    isPeerDevicePresent = 1;
    pthread_mutex_unlock (&PeerDevicePresence_mutex);
    
    pthread_cond_signal(&condition);
}

void onSnepClientClosed()
{
    printf("Peer device left\n-------------\n");

    pthread_mutex_lock (&PeerDevicePresence_mutex);
    isPeerDevicePresent = 0;
    pthread_mutex_unlock (&PeerDevicePresence_mutex);
}

int main(int argc, char ** argv) {	
    unsigned char NDEFMsg[100];
	unsigned int NDEFMsgLen = 0;

    pthread_mutex_init (&PeerDevicePresence_mutex, NULL);

    g_SnepServerCB.onMessageReceived = onMessageReceived;
    g_SnepClientCB.onDeviceArrival = onSnepClientReady;
    g_SnepClientCB.onDeviceDeparture = onSnepClientClosed;

    nfcManager_doInitialize();
    nfcSnep_registerClientCallback(&g_SnepClientCB);
    nfcSnep_startServer(&g_SnepServerCB);

    NDEFMsgLen = ndef_createText("en", "Hello world !", NDEFMsg, sizeof(NDEFMsg));
	if(NDEFMsgLen <= 0x00)
	{
		printf("Failed to build TEXT NDEF Message\n");
        exit(0);
	}	

    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0, 0, 0);

    printf("\n-------------\nWaiting for peer device ...\n");
    do{
        pthread_cond_wait(&condition, &mutex);

        printf("Waiting for 3 seconds to allow peer device to send NDEF message\n");
        sleep(3);

        if(isPeerDevicePresent)
        {
            if(nfcSnep_putMessage(NDEFMsg, NDEFMsgLen) == 0x00)
            {
                printf("NDEF message successfully pushed\n");
            }
            else
            {
                printf("NDEF message push failed\n");
            }
        }
    }while(1);

    nfcSnep_stopServer();
    nfcManager_disableDiscovery();
    nfcSnep_deregisterClientCallback();
    nfcManager_doDeinitialize();
}
