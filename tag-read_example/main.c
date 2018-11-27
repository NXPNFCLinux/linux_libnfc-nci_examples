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
    int i = 0;
    printf("UID: ");
    for(i = 0x00; i < (*pTagInfo).uid_length; i++){
        printf("%02X ", (unsigned char) (*pTagInfo).uid[i]);
    }
    printf("\n");
    g_tagInfos = *pTagInfo;
    pthread_cond_signal(&condition);
}

void onTagDeparture(void){
    printf("Tag removed\n-------------\n");
    printf("\n-------------\nWaiting for tag...\n");    
}

int main(int argc, char ** argv) {
    int res = 0x00;
    ndef_info_t NDEFinfo;
    unsigned char* NDEFContent = NULL;
    nfc_friendly_type_t lNDEFType = NDEF_FRIENDLY_TYPE_OTHER;
    char* TextContent = NULL;
    
    g_TagCB.onTagArrival = onTagArrival;
    g_TagCB.onTagDeparture = onTagDeparture;
    nfcManager_doInitialize();
    nfcManager_registerTagCallback(&g_TagCB);
    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x01, 0, 0);

    printf("\n-------------\nWaiting for tag...\n");
    do{
        pthread_cond_wait(&condition, &mutex);

        res = nfcTag_isNdef(g_tagInfos.handle, &NDEFinfo);
        if(0x01 == res) {
            NDEFContent = malloc(NDEFinfo.current_ndef_length * sizeof(unsigned char));
            res = nfcTag_readNdef(g_tagInfos.handle, NDEFContent, NDEFinfo.current_ndef_length, &lNDEFType);

            if(lNDEFType == NDEF_FRIENDLY_TYPE_TEXT) {
                TextContent = malloc(res * sizeof(char) + 1);
                res = ndef_readText(NDEFContent, res, TextContent, res);
                if(0x00 <= res)
                {
                    TextContent[res] = '\0';                    
                    printf("Text:  '%s'\n", TextContent);
                }
                else
                {
                    printf("Read NDEF Text Error\n");
                }
            }
            else {
                printf("Not a NDEF Text record\n");
            }
        }
        else {
            printf("Not a NDEF tag\n");
        }
    }while(1);
    nfcManager_doDeinitialize();
}
