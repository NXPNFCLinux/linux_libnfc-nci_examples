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

/********************************** HCE **********************************/
typedef enum T4T_NDEF_EMU_state_t
{
    Ready,
    NDEF_Application_Selected,
    CC_Selected,
    NDEF_Selected
} T4T_NDEF_EMU_state_t;
typedef void T4T_NDEF_EMU_Callback_t (unsigned char*, unsigned short);
const unsigned char T4T_NDEF_EMU_APP_Select[] = {0x00,0xA4,0x04,0x00,0x07,0xD2,0x76,0x00,0x00,0x85,0x01,0x01};
const unsigned char T4T_NDEF_EMU_CC[] = {0x00, 0x0F, 0x20, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0x06, 0xE1, 0x04, 0x00, 0xFF, 0x00, 0xFF};
const unsigned char T4T_NDEF_EMU_CC_Select[] = {0x00,0xA4,0x00,0x0C,0x02,0xE1,0x03};
const unsigned char T4T_NDEF_EMU_NDEF_Select[] = {0x00,0xA4,0x00,0x0C,0x02,0xE1,0x04};
const unsigned char T4T_NDEF_EMU_Read[] = {0x00,0xB0};
const unsigned char T4T_NDEF_EMU_OK[] = {0x90, 0x00};
const unsigned char T4T_NDEF_EMU_NOK[] = {0x6A, 0x82};
unsigned char *pT4T_NdefRecord = NULL;
unsigned short T4T_NdefRecord_size = 0;
static T4T_NDEF_EMU_state_t eT4T_NDEF_EMU_State = Ready;
static T4T_NDEF_EMU_Callback_t *pT4T_NDEF_EMU_PushCb = NULL;

static void T4T_NDEF_EMU_FillRsp (unsigned char *pRsp, unsigned short offset, unsigned char length)
{
    if (offset == 0)
    {
        pRsp[0] = (T4T_NdefRecord_size & 0xFF00) >> 8;
        pRsp[1] = (T4T_NdefRecord_size & 0x00FF);
        memcpy(&pRsp[2], &pT4T_NdefRecord[0], length-2);
    }
    else if (offset == 1)
    {
        pRsp[0] = (T4T_NdefRecord_size & 0x00FF);
        memcpy(&pRsp[1], &pT4T_NdefRecord[0], length-1);
    }
    else
    {
        memcpy(pRsp, &pT4T_NdefRecord[offset-2], length);
    }
    /* Did we reached the end of NDEF record ?*/
    if ((offset + length) >= (T4T_NdefRecord_size + 2))
    {
        /* Notify application of the NDEF send */
        if(pT4T_NDEF_EMU_PushCb != NULL) pT4T_NDEF_EMU_PushCb(pT4T_NdefRecord, T4T_NdefRecord_size);
    }
}

void T4T_NDEF_EMU_SetRecord(unsigned char *pRecord, unsigned short Record_size, T4T_NDEF_EMU_Callback_t *cb)
{
    pT4T_NdefRecord = pRecord;
    T4T_NdefRecord_size = Record_size;
    pT4T_NDEF_EMU_PushCb =  cb;
}

void T4T_NDEF_EMU_Reset(void)
{
    eT4T_NDEF_EMU_State = Ready;
}

void T4T_NDEF_EMU_Next(unsigned char *pCmd, unsigned short Cmd_size, unsigned char *pRsp, unsigned short *pRsp_size)
{
    unsigned char eStatus = 0x00;
    if (!memcmp(pCmd, T4T_NDEF_EMU_APP_Select, sizeof(T4T_NDEF_EMU_APP_Select)))
    {
        *pRsp_size = 0;
        eStatus = 0x01;
        eT4T_NDEF_EMU_State = NDEF_Application_Selected;
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_CC_Select, sizeof(T4T_NDEF_EMU_CC_Select)))
    {
        if(eT4T_NDEF_EMU_State == NDEF_Application_Selected)
        {
            *pRsp_size = 0;
            eStatus = 0x01;
            eT4T_NDEF_EMU_State = CC_Selected;
        }
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_NDEF_Select, sizeof(T4T_NDEF_EMU_NDEF_Select)))
    {
        *pRsp_size = 0;
        eStatus = 0x01;
        eT4T_NDEF_EMU_State = NDEF_Selected;
    }
    else if (!memcmp(pCmd, T4T_NDEF_EMU_Read, sizeof(T4T_NDEF_EMU_Read)))
    {
        if(eT4T_NDEF_EMU_State == CC_Selected)
        {
            memcpy(pRsp, T4T_NDEF_EMU_CC, sizeof(T4T_NDEF_EMU_CC));
            *pRsp_size = sizeof(T4T_NDEF_EMU_CC);
            eStatus = 0x01;
        }
        else if (eT4T_NDEF_EMU_State == NDEF_Selected)
        {
            unsigned short offset = (pCmd[2] << 8) + pCmd[3];
            unsigned char length = pCmd[4];
            if(length <= (T4T_NdefRecord_size + offset + 2))
            {
                T4T_NDEF_EMU_FillRsp(pRsp, offset, length);
                *pRsp_size = length;
                eStatus = 0x01;
            }
        }
    }
    if (eStatus == 0x01)
    {
        memcpy(&pRsp[*pRsp_size], T4T_NDEF_EMU_OK, sizeof(T4T_NDEF_EMU_OK));
        *pRsp_size += sizeof(T4T_NDEF_EMU_OK);
    }
    else
    {
        memcpy(pRsp, T4T_NDEF_EMU_NOK, sizeof(T4T_NDEF_EMU_NOK));
        *pRsp_size = sizeof(T4T_NDEF_EMU_NOK);
        T4T_NDEF_EMU_Reset();
    }
}
/*************************************************************************/

void onDataReceived(unsigned char *data, unsigned int data_length)
{
    HCE_dataLength = data_length;
    HCE_data = malloc(HCE_dataLength * sizeof(unsigned char));
    memcpy(HCE_data, data, data_length);

    pthread_cond_signal(&condition);
}

void onHostCardEmulationActivated(unsigned char mode)
{
    printf("-------------\nCard activated\n");
    T4T_NDEF_EMU_Reset();
}

void onHostCardEmulationDeactivated(void)
{
    printf("Card deactivated\n-------------\n\nWaiting for reader...\n\n");    
}

void ndefEmulationPushed (unsigned char *msg, unsigned short msg_length)
{
    printf("NDEF message pushed\n");
}

int main(int argc, char ** argv) {
    int res;
    unsigned char HCE_response[255];
    unsigned short HCE_response_len;
    unsigned char NDEFMsg[100];
    unsigned int NDEFMsgLen = 0;

    g_HceCB.onDataReceived = onDataReceived;
    g_HceCB.onHostCardEmulationActivated = onHostCardEmulationActivated;
    g_HceCB.onHostCardEmulationDeactivated = onHostCardEmulationDeactivated;

    nfcManager_doInitialize();
    nfcHce_registerHceCallback(&g_HceCB);
    nfcManager_enableDiscovery(0x00, 0, 1, 0);

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

    T4T_NDEF_EMU_SetRecord(NDEFMsg, NDEFMsgLen, ndefEmulationPushed);

    printf("\n-------------\nWaiting for reader...\n\n");
    do{
        /* Wait for data from remote reader */
        pthread_cond_wait(&condition, &mutex);

        if(HCE_data != NULL)
        {
            /* Call HCE response builder */
            T4T_NDEF_EMU_Next(HCE_data, HCE_dataLength, HCE_response, &HCE_response_len);

            if(HCE_response_len > 0)
            {
                nfcHce_sendCommand(HCE_response, HCE_response_len);
            }
        }
    }while(1);
}
