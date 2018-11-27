#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "linux_nfc_api.h"

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

nfcHostCardEmulationCallback_t g_HceCB;
unsigned char *HCE_data = NULL;
unsigned int HCE_dataLength = 0x00;
unsigned char g_readerMode = 0x00;

void onDataReceived(unsigned char *data, unsigned int data_length)
{
    printf("Data received\n");

    HCE_dataLength = data_length;
    HCE_data = malloc(HCE_dataLength * sizeof(unsigned char));
    memcpy(HCE_data, data, data_length);

    pthread_cond_signal(&condition);
}

void onHostCardEmulationActivated(unsigned char mode)
{
    printf("-------------\nCard activated - ");
    g_readerMode = mode;
    switch(mode)
    {
        case MODE_LISTEN_A: printf("Remote reader is type A\n");
            break;
        case MODE_LISTEN_B: printf("Remote reader is type B\n");
            break;
        case MODE_LISTEN_F: printf("Remote reader is type F\n");
            break;
        default: printf(" Remote reader type is unknown\n");
            break;
    }
}

void onHostCardEmulationDeactivated()
{
    printf("Card deactivated\n-------------\n");
    g_readerMode = 0x00;
}

int main(int argc, char ** argv) {
    int i;
    int res;
    unsigned char T3tId[]={/* system code*/ 0x01,0xfc, /* nfcid2 */ 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    unsigned char HCE_response[255];
    unsigned char HCE_response_len;

    g_HceCB.onDataReceived = onDataReceived;
    g_HceCB.onHostCardEmulationActivated = onHostCardEmulationActivated;
    g_HceCB.onHostCardEmulationDeactivated = onHostCardEmulationDeactivated;

    nfcManager_doInitialize();
    nfcHce_registerHceCallback(&g_HceCB);
    nfcHce_registerT3tIdentifier(T3tId, sizeof(T3tId));
    nfcManager_enableDiscovery(0x00, 0, 1, 0);

    printf("\n-------------\nWaiting for reader...\n");
    do{
        /* Wait for data from remote reader */
        pthread_cond_wait(&condition, &mutex);

    if(HCE_data != NULL)
    {
        printf("\t\tReceived data from remote device : \n\t\t");
        for(i = 0x00; i < HCE_dataLength; i++)
        {
            printf("%02X ", HCE_data[i]);
        }

        switch (g_readerMode)
        {
            case MODE_LISTEN_A:
            case MODE_LISTEN_B:
                /* Just reply with status OK */
                HCE_response[0] = 0x90;
                HCE_response[1] = 0x00;
                HCE_response_len = 2;
                break;

            case MODE_LISTEN_F:
                /* Just loopback */
                memcpy(HCE_response, HCE_data, HCE_dataLength);
                HCE_response_len = HCE_dataLength;
                break;

            default:
                res = 0x00;
                break;
            }

            if(HCE_response_len > 0)
            {
                res = nfcHce_sendCommand(HCE_response, HCE_response_len);
            }
            else
            {
                res = 0;
            }

            if(0x00 == res)
            {
                printf("\n\n\t\tResponse sent : \n\t\t");
                for(i = 0x00; i < HCE_response_len; i++)
                {
                    printf("%02X ", HCE_response[i]);
                }
                printf("\n\n");
            }
            else
            {
                printf("\n\n\t\tFailed to send response\n\n");
            }
        }
    }while(1);
}
