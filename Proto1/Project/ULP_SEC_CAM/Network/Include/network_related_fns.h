

#ifndef NETWORK_RELATED_FNS_H_
#define NETWORK_RELATED_FNS_H_

#include "simplelink.h"

#define AP_SSID_LEN_MAX         (33)
#define AP_PASSWORD_LEN_MAX     (50)
#define AP_SECTYPE_LEN_MAX		(2)

#define ROLE_INVALID            (-5)

#define CONNECTION_TIMEOUT_COUNT  5000  /* 5sec */
#define TOKEN_ARRAY_SIZE          6
//#define TOKEN_ARRAY_SIZE          1
#define STRING_TOKEN_SIZE         10
#define SCAN_TABLE_SIZE           20

extern unsigned long  g_ulStatus;//SimpleLink Status
extern unsigned long  g_ulGatewayIP; //Network Gateway IP address
extern unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
extern unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
extern signed char g_cWlanSSID[AP_SSID_LEN_MAX];
extern int g_uiSimplelinkRole;
extern signed char g_cWlanSecurityType[2];
extern signed char g_cWlanSecurityKey[50];
extern SlSecParams_t g_SecParams;
extern unsigned char g_ucProfileAdded;
extern unsigned char g_ucConnectedToConfAP, g_ucProvisioningDone;
extern unsigned char g_ucPriority;

extern Sl_WlanNetworkEntry_t g_NetEntries[SCAN_TABLE_SIZE];
extern char g_token_get [TOKEN_ARRAY_SIZE][STRING_TOKEN_SIZE];

void InitializeAppVariables();	//Shift fn to suitable place


long ConfigureSimpleLinkToDefaultState();
long ConnectToNetwork();
int ConfigureMode(int iMode);

int32_t ConnectToNetwork_STA();

void ConnectToNetwork_STA_2();


#endif /* NETWORK_RELATED_FNS_H_ */
