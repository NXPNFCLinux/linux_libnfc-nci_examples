#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "linux_nfc_api.h"

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

nfcTagCallback_t g_TagCB;
nfc_tag_info_t g_tagInfos;

void onTagArrival(nfc_tag_info_t *pTagInfo){
    printf("Tag detected\n");
    g_tagInfos = *pTagInfo;
    pthread_cond_signal(&condition);
}

void onTagDeparture(void){
    printf("Tag removed\n-------------\n");
}

int main(int argc, char ** argv) {
	unsigned char NDEFMsg[100];
	unsigned int NDEFMsgLen = 0;
	int res;
	ndef_info_t NDEFinfo;

    g_TagCB.onTagArrival = onTagArrival;
    g_TagCB.onTagDeparture = onTagDeparture;
    nfcManager_doInitialize();
    nfcManager_registerTagCallback(&g_TagCB);
    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x01, 0, 0);

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

    printf("\n-------------\nWaiting for tag...\n");
    do{
        pthread_cond_wait(&condition, &mutex);

		if(nfcTag_isNdef(g_tagInfos.handle, &NDEFinfo))
        {
            printf("Tag already NDEF formatted\n");
        }
        else
        {
            printf("Tag not yet NDEF formatted, trying to format\n");
            if(nfcTag_isFormatable(g_tagInfos.handle))
            {
                if(nfcTag_formatTag(g_tagInfos.handle) == 0x00)
                {
		            if(nfcTag_isNdef(g_tagInfos.handle, &NDEFinfo))
                    {
                    	printf("Tag formating succeed\n");
                    }
                    else
                    {
        				printf("Tag formating failed\n");
                    }
                }
                else
                {
    				printf("Tag formating failed\n");
                }
            }
            else
            {
                printf("Tag is not formatable\n");
            }
        }
 
    	if(nfcTag_writeNdef(g_tagInfos.handle, NDEFMsg, NDEFMsgLen) == 0x00)
        {
           	printf("Write succeed\n");
        }
        else
        {
			printf("Write failed\n");
        }
    }while(1);
    nfcManager_doDeinitialize();
}
