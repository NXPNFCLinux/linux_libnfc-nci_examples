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
    g_TagCB.onTagArrival = onTagArrival;
    g_TagCB.onTagDeparture = onTagDeparture;
    nfcManager_doInitialize();
    nfcManager_registerTagCallback(&g_TagCB);
    nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0x01, 0, 0);

    int tag_count=0;
    int num_tags = 0;
    unsigned int i;

    printf("\n-------------\nWaiting for tags...\n");
    do{
        pthread_cond_wait(&condition, &mutex);

        num_tags = nfcManager_getNumTags();
        printf("Tag UID =");
        for(i=0; i<g_tagInfos.uid_length; i++) printf("%.2X", g_tagInfos.uid[i]);
        printf("\n");
        if(num_tags > 1)
        {
            tag_count++;
            if (tag_count < num_tags)
            {
                printf("selecting next tag...\n");
                nfcManager_selectNextTag();
            }
            else
            {
                tag_count = 0;
            }
        }
    }while(1);
    nfcManager_doDeinitialize();
}
