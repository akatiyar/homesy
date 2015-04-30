
// Simplelink includes
#include "simplelink.h"

#include "app.h"
#include "network_related_fns.h"

#include "camera_app.h"

unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
extern int g_uiSimplelinkRole = ROLE_INVALID;

extern volatile unsigned char g_CaptureImage;

void initNetwork(signed char *ssid, SlSecParams_t *keyParams);


//****************************************************************************
//
//!    \brief This function initializes the application variables
//!
//!    \param[in]  None
//!
//!    \return     0 on success, negative error-code on error
//
//****************************************************************************
void InitializeAppVariables()
{
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
}

//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();

    return lRetVal; // Success
}

//****************************************************************************
//
//! Confgiures the mode in which the device will work
//!
//! \param iMode is the current mode of the device
//!
//!
//! \return   SlWlanMode_t
//!
//
//****************************************************************************
int ConfigureMode(int iMode)
{
    long   lRetVal = -1;

    lRetVal = sl_WlanSetMode(iMode);
    ASSERT_ON_ERROR(lRetVal);

    /* Restart Network processor */
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    // reset status bits
    CLR_STATUS_BIT_ALL(g_ulStatus);

    return sl_Start(NULL,NULL,NULL);
}

//*****************************************************************************
//
//!    ConnectToNetwork
//!    Setup SimpleLink in AP Mode
//!
//!    \param                      None
//!     \return                     0 - Success
//!                                   Negative - Failure
//!
//
//*****************************************************************************
long ConnectToNetwork()
{
    long lRetVal = -1;

    //Start Simplelink Device
    lRetVal =  sl_Start(NULL,NULL,NULL);
    ASSERT_ON_ERROR(lRetVal);

    // Device is in STA mode, Switch to AP Mode
    if(lRetVal == ROLE_STA)
    {
        //
        // Configure to AP Mode
        //
        if(ConfigureMode(ROLE_AP) !=ROLE_AP)
        {
            UART_PRINT("Unable to set AP mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            ASSERT_ON_ERROR(DEVICE_NOT_IN_AP_MODE);
        }
    }

    while(!IS_IP_ACQUIRED(g_ulStatus))
    {
      //looping till ip is acquired
    }

     //Read the AP SSID
    unsigned char ucAPSSID[AP_SSID_LEN_MAX];
    unsigned short len = 32;
    unsigned short  config_opt = WLAN_AP_OPT_SSID;
    memset(ucAPSSID,'\0',AP_SSID_LEN_MAX);
    lRetVal = sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, (unsigned char*) ucAPSSID);
    ASSERT_ON_ERROR(lRetVal);

    //Stop Internal HTTP Server
    lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
    ASSERT_ON_ERROR(lRetVal);

    //Start Internal HTTP Server
    lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
    ASSERT_ON_ERROR(lRetVal);
    return SUCCESS;
}

long ConnectToNetwork_2()
{
    long lRetVal = -1;
    //unsigned int uiConnectTimeoutCnt =0; //uncomment if wifi details are not hard-coded

    //Start Simplelink Device
    lRetVal =  sl_Start(NULL,NULL,NULL);
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_WlanSetMode(ROLE_STA);
	if (lRetVal < 0) {
		sl_Stop(SL_STOP_TIMEOUT);
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}

		SlSecParams_t secParams = {0};
		lRetVal = 0;

		secParams.Key = (signed char*)SECURITY_KEY;
		secParams.KeyLen = strlen(SECURITY_KEY);
		secParams.Type = SECURITY_TYPE;

		lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
		ASSERT_ON_ERROR(lRetVal);

		// Wait for WLAN Event
		while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
		{
#ifdef LED_INDICATION
			// Toggle LEDs to Indicate Connection Progress
			GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
			MAP_UtilsDelay(800000);
			GPIO_IF_LedOn(MCU_IP_ALLOC_IND);
			MAP_UtilsDelay(800000);
#endif
		}

		return SUCCESS;
}

void ConnectToNetwork_STA_2()
{
	 long lRetVal = -1;

	//Initialize Global Variable
	InitializeAppVariables();

	ConfigureSimpleLinkToDefaultState();

	//Connect to Network
	lRetVal = ConnectToNetwork_2();
	if(lRetVal < 0)
	{
		UART_PRINT("Failed to establish connection w/ an AP \n\r");
		LOOP_FOREVER();
	}
}

void ConnectToNetwork_STA()
{
	// TODO: Change to your SSID and password
	signed char ssid[] = APP_SSID_NAME;
	signed char key[] = APP_SSID_PASSWORD;
	SlSecParams_t keyParams;
	keyParams.Type = APP_SSID_SEC_TYPE;
	keyParams.Key = key;
	keyParams.KeyLen = strlen((char *)key);

	initNetwork(ssid, &keyParams);

	// TODO: Set to current date/time (within an hour precision)
	SlDateTime_t dateTime;
	memset(&dateTime, 0, sizeof(dateTime));
	dateTime.sl_tm_year = 2015;
	dateTime.sl_tm_mon = 4;
	dateTime.sl_tm_day = 30;
	dateTime.sl_tm_hour = 19;
	sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION, SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME, sizeof(SlDateTime_t), (unsigned char *)&dateTime);
}

void initNetwork(signed char *ssid, SlSecParams_t *keyParams)
{
	short status = sl_Start(0, 0, 0);
	if (status >= 0)
	{
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
	}

	if (status < 0) {
		sl_Stop(SL_STOP_TIMEOUT);
		ERR_PRINT(status);
		LOOP_FOREVER();
	}

	if (status < 0) {
		sl_Stop(SL_STOP_TIMEOUT);
		ERR_PRINT(status);
		LOOP_FOREVER();
	}

	sl_WlanDisconnect();

	status = sl_WlanSetMode(ROLE_STA);
	if (status < 0) {
		sl_Stop(SL_STOP_TIMEOUT);
		ERR_PRINT(status);
		LOOP_FOREVER();
	}

	UART_PRINT("\r\n");
	UART_PRINT("[QuickStart] Network init\r\n");

	status = sl_WlanConnect(ssid, strlen((char *)ssid), NULL, keyParams, NULL);
	if (status < 0) {
		sl_Stop(SL_STOP_TIMEOUT);
		ERR_PRINT(status);
		LOOP_FOREVER();
	}

	while (!IS_IP_ACQUIRED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
		_SlNonOsMainLoopTask();
#else
		osi_Sleep(100);
#endif
	}

	sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'-Applications
            // can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s ,\
                BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      g_ucConnectionSSID,g_ucConnectionBSSID[0],
                      g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                      g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                      g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        default:
        {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
            "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
        }
        break;

        default:
        {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    //
    // This application doesn't work w/ socket - Events are not expected
    //
       switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->EventData.status )
            {
                case SL_ECLOSE:
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                    "failed to transmit all queued packets\n\n",
                           pSock->EventData.sd);
                    break;
                default:
                    UART_PRINT("[SOCK ERROR] - TX FAILED : socket %d , reason"
                        "(%d) \n\n",
                           pSock->EventData.sd, pSock->EventData.status);
            }
            break;

        default:
            UART_PRINT("[SOCK EVENT] "
            			"- Unexpected Event [%x0x]\n\n",pSock->Event);
    }
    UART_PRINT("[SOCK EVENT/ERROR]:\n Event: %x\n Socket: %x\n EventStatus: "
    				"%x\n AsynchEventValue: %x\n AsynchEventType: %x\n\r",
    				pSock->Event, pSock->EventData.sd, pSock->EventData.status,
    				pSock->EventData.socketAsyncEvent.val,
    				pSock->EventData.socketAsyncEvent.type);
}

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent,
                               SlHttpServerResponse_t *pSlHttpServerResponse)
{
    if((pSlHttpServerEvent == NULL) || (pSlHttpServerResponse == NULL))
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    switch (pSlHttpServerEvent->Event)
    {
       case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
       	   {
              if (0 == memcmp (pSlHttpServerEvent->
                       EventData.httpPostData.token_name.data, "__SL_P_U.C",
                    pSlHttpServerEvent->EventData.httpPostData.token_name.len))
              {
            	  if(0 == memcmp (pSlHttpServerEvent->EventData.httpPostData.token_value.data, \
            	                                    "start", \
            	                     pSlHttpServerEvent->EventData.httpPostData.token_value.len))
            	  {
            		  g_CaptureImage = 1;
            	  }
            	  else
            	  {
            		  g_CaptureImage = 0;
            	  }
              }
       	   }
        	break;
       default:
    	   break;
    }
}
