//#include "wlan.h"
#include "wifi_provisioing_thruAPmode.h"
#include "network_related_fns.h"
#include "flash_files.h"

#include "simplejson.h"

static long WlanConnect();
//****************************************************************************
//
//! Task function implements the ProvisioningAP functionality
//!
//! \param none
//!
//! This function
//!    3. Switch to AP Mode and Wait for AP Configuration from Browser
//!    4. Switch to STA Mode and Connect to Configured AP
//!
//! \return None.
//
//****************************************************************************
int32_t ProvisioningAP()
{
   long lRetVal = -1;

    InitializeAppVariables();

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
           UART_PRINT("Failed to configure the device in its default state\n\r");
       }

       LOOP_FOREVER();
   }
    UART_PRINT("Device is configured in default state \n\r");

	//	Start NWP
	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0)
	{
	   UART_PRINT("Failed to start the device \n\r");
	   LOOP_FOREVER();
	}

	while(!g_ucProvisioningDone)
    {
    	UART_PRINT("Top of while()\n\r");

        //
        // Configure to AP Mode
        //
        if(ConfigureMode(ROLE_AP) !=ROLE_AP)
        {
            UART_PRINT("Unable to set AP mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            LOOP_FOREVER();
        }
        while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
        UART_PRINT("Configd APmode\n\r");

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
        UART_PRINT("HTTPSrvr Strtd\n\r");

        //
		// Wait for AP Configuraiton, Open Browser and Configure AP
		//
		while(g_ucProfileAdded)
		{
			MAP_UtilsDelay(100);
			//UART_PRINT("w");
		}
		UART_PRINT("\n\rwhile exit\n\r");

		g_ucProfileAdded = 1;

		//
		//	Test connection
		//
        // Configure to STA Mode
        if(ConfigureMode(ROLE_STA) !=ROLE_STA)
        {
            UART_PRINT("Unable to set STA mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            LOOP_FOREVER();
        }
        // Connect to the Configured Access Point
        lRetVal = WlanConnect();
		if(lRetVal == SUCCESS)
		{
			g_ucProvisioningDone = 1;	//Set the global variable
			g_ucConnectedToConfAP = IS_CONNECTED(g_ulStatus);

			//Disconnect back
			UART_PRINT("WLAN Connect Done\n\r");
			lRetVal = sl_WlanDisconnect();
			if(0 == lRetVal)
			{
				while(IS_CONNECTED(g_ulStatus));
			}
			UART_PRINT("WLAN Disconnected back\n\r");

			//Write to Flash file
			lRetVal = CreateFile_Flash(FILENAME_USERWIFI, MAX_FILESIZE_USERWIFI);
			ASSERT_ON_ERROR(lRetVal);
			uint8_t ucConfigFileData[CONTENTSIZE_FILE_USERWIFI];
			uint8_t* pucConfigFileData = &ucConfigFileData[0];

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSSID);
			pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityType);
			pucConfigFileData += strlen((const char*)pucConfigFileData)+1;

			strcpy((char*)pucConfigFileData, (const char*)g_cWlanSecurityKey);

			int32_t lFileHandle;
			lRetVal = WriteFile_ToFlash((uint8_t*)&ucConfigFileData[0],
										(uint8_t*)FILENAME_USERWIFI,
										CONTENTSIZE_FILE_USERWIFI,
										0, 1, &lFileHandle);
			ASSERT_ON_ERROR(lRetVal);

			// For verification - DBG
			lRetVal = ReadFile_FromFlash((ucConfigFileData+3),
											(uint8_t*)FILENAME_USERWIFI,
											CONTENTSIZE_FILE_USERWIFI-3,
											0);
			ASSERT_ON_ERROR(lRetVal);

			/*lRetVal = WriteFile_ToFlash((uint8_t*)&g_cWlanSSID[0], (uint8_t*)FILENAME_USERWIFI,
										AP_SSID_LEN_MAX, 0);
			lRetVal = WriteFile_ToFlash((uint8_t*)(&(g_SecParams.Type)), (uint8_t*)FILENAME_USERWIFI,
										AP_SECTYPE_LEN_MAX, AP_SSID_LEN_MAX);
			lRetVal = WriteFile_ToFlash((uint8_t*)g_SecParams.Key, (uint8_t*)FILENAME_USERWIFI,
										AP_PASSWORD_LEN_MAX,
										AP_SSID_LEN_MAX + AP_SECTYPE_LEN_MAX);*/

			/*uint8_t ucConfigFileData[FILE_SIZE_USER_CONFIG];
			uint8_t ucSSID[33];
			uint8_t uc[33];
			ReadFile_FromFlash(ucConfigFileData, FILE_NAME_USER_CONFIG, FILE_SIZE_USER_CONFIG, 0);
			simpleJsonProcessor(ucConfigFileData, "SSID", ucSSID, SSID_LEN_MAX+1);*/
		}

		//
		// Configure to AP Mode to display message
		//
		if(ConfigureMode(ROLE_AP) !=ROLE_AP)
		{
			UART_PRINT("Unable to set AP mode...\n\r");
			lRetVal = sl_Stop(SL_STOP_TIMEOUT);
			CLR_STATUS_BIT_ALL(g_ulStatus);
			LOOP_FOREVER();
		}
		while(!IS_IP_ACQUIRED(g_ulStatus));	//looping till ip is acquired
		UART_PRINT("Configd APmode\n\r");

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
		UART_PRINT("HTTPSrvr Strtd again\n\r");
		MAP_UtilsDelay(80000000);	//Wait for some time to let http msgs txit
    }

	//Stop Internal HTTP Server
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

	lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	CLR_STATUS_BIT_ALL(g_ulStatus);

    return SUCCESS;
}

//******************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  0 - Success
//!            -1 - Failure

//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//******************************************************************************
static long WlanConnect()
{
	long lRetVal = -1;
	unsigned int uiConnectTimeoutCnt =0;

    //Connecting to the Access point
	lRetVal = sl_WlanConnect(g_cWlanSSID, strlen((char*)g_cWlanSSID), 0,
								&g_SecParams, 0);
	ASSERT_ON_ERROR(lRetVal);

    // Wait for WLAN Event
    while(uiConnectTimeoutCnt<CONNECTION_TIMEOUT_COUNT &&
		((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))))
    {
        osi_Sleep(1); //Sleep 1 millisecond
        uiConnectTimeoutCnt++;
    }

    if (!(IS_CONNECTED(g_ulStatus) || IS_IP_ACQUIRED(g_ulStatus)))
    {
    	return USER_WIFI_PROFILE_FAILED_TO_CONNECT;
    }
    else
    {
    	return SUCCESS;
    }
}
