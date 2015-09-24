

#ifndef NETWORK_RELATED_FNS_H_
#define NETWORK_RELATED_FNS_H_

#include "nwp.h"
#include "simplelink.h"
#include "app.h"

#define AP_SSID_LEN_MAX         (33)
#define AP_PASSWORD_LEN_MAX     (50)
#define AP_SECTYPE_LEN_MAX		(2)

#define ROLE_INVALID            (-5)

//#define CONNECTION_TIMEOUT_COUNT  5000  /* 5sec when while loop has osi_Sleep(1)*/
#define CONNECTION_TIMEOUT_COUNT  6000  /* 60sec when while loop has osi_Sleep(10)*/
#define TOKEN_ARRAY_SIZE          6
#define STRING_TOKEN_SIZE         10
#define SCAN_TABLE_SIZE           20

#define TOK_CALIB				"__SL_P_UCB"
#define TOK_ANGLE				"__SL_P_UAG"
#define TOK_EXIT				"__SL_P_UEX"
#define TOK_KEYTYPE				"__SL_P_USE"
#define TOK_SSID				"__SL_P_USD"
#define TOK_KEY					"__SL_P_USF"
#define TOK_CONFIG_WIFI			"__SL_P_US0"
#define TOK_ACTION1				"__SL_P_UA1"
#define TOK_ACTION2				"__SL_P_UA2"
#define TOK_ACTION3				"__SL_P_UA3"
#define TOK_ACTION4				"__SL_P_UA4"
#define TOK_PREVIEW				"__SL_P_UPR"
#define TOK_AWB					"__SL_P_UAW"
#define TOK_SAVE				"__SL_P_USA"

#define TIME2013                3565987200u      /* 113 years + 28 days(leap) */
#define YEAR2013                2013
#define SEC_IN_MIN              60
#define SEC_IN_HOUR             3600
#define SEC_IN_DAY              86400

#define SERVER_RESPONSE_TIMEOUT 10
#define GMT_DIFF_TIME_HRS       5
#define GMT_DIFF_TIME_MINS      30

extern unsigned long  g_ulGatewayIP; //Network Gateway IP address
extern unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
extern unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
extern signed char g_cWlanSSID[AP_SSID_LEN_MAX];
extern int g_uiSimplelinkRole;
extern signed char g_cWlanSecurityType[2];
extern signed char g_cWlanSecurityKey[50];
extern SlSecParams_t g_SecParams;

unsigned char g_PhoneConnected_ToCC3200AP_flag;
extern unsigned char g_ucConnectedToConfAP, g_ucProvisioningDone;
extern unsigned char g_ucPriority;

extern Sl_WlanNetworkEntry_t g_NetEntries[SCAN_TABLE_SIZE];
extern char g_token_get [TOKEN_ARRAY_SIZE][STRING_TOKEN_SIZE];

long ConfigureSimpleLinkToDefaultState();
int ConfigureMode(int iMode);
int32_t WiFi_Connect();
int32_t AccessPtMode_HTTPServer_Start();
int32_t WiFiProvisioning();
int32_t GetTimeNTP(SlDateTime_t *dateTime);

#endif /* NETWORK_RELATED_FNS_H_ */
