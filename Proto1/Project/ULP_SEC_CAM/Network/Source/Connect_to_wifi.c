/*
 * connect_to_wifi.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <app_fns.h>
#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"
#include "camera_app.h"
#include "flash_files.h"
#include "mt9d111.h"

static int32_t initNetwork(signed char *ssid, SlSecParams_t *keyParams);

int32_t WiFi_Connect()
{
	int32_t lRetVal;
	uint8_t ucWifiConfigFileData[WIFI_DATA_SIZE];
	int8_t ssid[AP_SSID_LEN_MAX];
	int8_t ucWifiPassword[AP_PASSWORD_LEN_MAX];
	uint8_t ucWifiSecType[AP_SECTYPE_LEN_MAX];
	uint8_t* pucWifiConfigFileData;// = &ucWifiConfigFileData[0];
	SlSecParams_t keyParams;

	ConfigureSimpleLinkToDefaultState();
	//lRetVal = sl_Start(0, 0, 0);
	lRetVal = NWP_SwitchOn();
	RETURN_ON_ERROR(lRetVal);

	lRetVal = ReadFile_FromFlash(ucWifiConfigFileData,
									(uint8_t*)USER_CONFIGS_FILENAME,
									WIFI_DATA_SIZE, WIFI_DATA_OFFSET);

	pucWifiConfigFileData = &ucWifiConfigFileData[0];
	RETURN_ON_ERROR(lRetVal);

	strcpy((char*)ssid, (const char*)ucWifiConfigFileData);
	pucWifiConfigFileData += strlen((const char*)pucWifiConfigFileData)+1;
	strcpy((char*)ucWifiSecType, (const char*)pucWifiConfigFileData);
	pucWifiConfigFileData += strlen((const char*)pucWifiConfigFileData)+1;
	strcpy((char*)ucWifiPassword, (const char*)pucWifiConfigFileData);

	keyParams.Type = ucWifiSecType[0];
	keyParams.Key = ucWifiPassword;
	keyParams.KeyLen = strlen((char *)ucWifiPassword);

	lRetVal = initNetwork(ssid, &keyParams);
	RETURN_ON_ERROR(lRetVal);

	// TODO: Set to current date/time (within an hour precision)
	SlDateTime_t dateTime;
	memset(&dateTime, 0, sizeof(dateTime));

	dateTime.sl_tm_year = 2015;
	dateTime.sl_tm_mon = 8;
	dateTime.sl_tm_day = 12;
	dateTime.sl_tm_hour = 10;

	GetTimeNTP(&dateTime);

	sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
				SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
				sizeof(SlDateTime_t), (unsigned char *)&dateTime);

	return lRetVal;
}

int32_t initNetwork(signed char *ssid, SlSecParams_t *keyParams)
{
	short status;
	unsigned int uiConnectTimeoutCnt =0;

	// disable scan
	unsigned char configOpt = SL_SCAN_POLICY(0);
	sl_WlanPolicySet(SL_POLICY_SCAN, configOpt, NULL, 0);

	// set tx power to maximum
	unsigned char txPower = 0;
	sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&txPower);

	// set power policy to normal
	sl_WlanPolicySet(SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0);

	// remove all rx filters
	_WlanRxFilterOperationCommandBuff_t rxFilterIdMask;
	memset(rxFilterIdMask.FilterIdMask, 0xFF, 8);
	sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (unsigned char *)&rxFilterIdMask, sizeof(_WlanRxFilterOperationCommandBuff_t));

	status = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 0), NULL, 0);
	RETURN_ON_ERROR(status);

	sl_WlanDisconnect();

	status = sl_WlanSetMode(ROLE_STA);
	RETURN_ON_ERROR(status);

	//DEBG_PRINT("\r\n[QuickStart] Network init\r\n");
	DEBG_PRINT("WiFi SSID:%s\nPassword:%s\nSecurity Type:%x\n", ssid, keyParams->Key, keyParams->Type);
	status = sl_WlanConnect(ssid, strlen((char *)ssid), NULL, keyParams, NULL);
	RETURN_ON_ERROR(status);

    // Wait for WLAN Event
    while(uiConnectTimeoutCnt<CONNECTION_TIMEOUT_COUNT &&
		((!IS_CONNECTED(g_ulSimplelinkStatus)) || (!IS_IP_ACQUIRED(g_ulSimplelinkStatus))))
    {
    	osi_Sleep(10);
        uiConnectTimeoutCnt++;
    }

    if(uiConnectTimeoutCnt >= CONNECTION_TIMEOUT_COUNT)
    {
    	status = USER_WIFI_PROFILE_FAILED_TO_CONNECT;
    }

	return ((int32_t)status);
}

