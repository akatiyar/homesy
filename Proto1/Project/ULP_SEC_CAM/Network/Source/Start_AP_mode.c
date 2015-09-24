/*
 * AP_mode.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "app_common.h"
#include "network_related_fns.h"

//******************************************************************************
//	Puts CC3200 in AP mode and starts its HTTP server
//******************************************************************************
int32_t AccessPtMode_HTTPServer_Start()
{
	int32_t lRetVal = -1;
	unsigned char ucFridgeCamID[50];
	unsigned short  length;

	//
	// Following function configure the device to default state by cleaning
	// the persistent settings stored in NVMEM (viz. connection profiles &
	// policies, power policy etc)
	//
	// Applications may choose to skip this step if the developer is sure
	// that the device is in its default state at start of applicaton
	//
	// Note that all profiles and persistent settings that were done on the
	// device will be lost
	//
   lRetVal = ConfigureSimpleLinkToDefaultState();
   if(lRetVal < 0)
   {
	   if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
	   {
		   DEBG_PRINT("Failed to configure NWP in default state\n");
	   }

	   LOOP_FOREVER();
   }
	//DEBG_PRINT("Device is configured in default state \n\r");

	//	Start NWP
	//lRetVal = sl_Start(0, 0, 0);
    lRetVal = NWP_SwitchOn();
	if (lRetVal < 0)
	{
	   DEBG_PRINT("Failed to start NWP\n");
	   LOOP_FOREVER();
	}

    // Set SSID name for AP mode
    Get_FridgeCamID(&ucFridgeCamID[0]);
	length = strlen((const char *)&ucFridgeCamID[0]);
	lRetVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, length, &ucFridgeCamID[0]);

	//
	// Configure to AP Mode
	//
	if(ConfigureMode(ROLE_AP) !=ROLE_AP)
	{
		DEBG_PRINT("Unable to set AP mode...\n");
		lRetVal = sl_Stop(SL_STOP_TIMEOUT);
		//CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		g_ulSimplelinkStatus = 0;
		DEBG_PRINT("SL3: %x\n", g_ulSimplelinkStatus);
		LOOP_FOREVER();
	}
	while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus));	//looping till ip is acquired
	DEBG_PRINT("CC3200 in APmode\n");

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}
	//Start Internal HTTP Server
	lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	DEBG_PRINT("HTTPServer Started\n");

	return lRetVal;
}
