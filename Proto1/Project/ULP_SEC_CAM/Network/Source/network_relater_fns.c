
// Simplelink includes
#include "app_common.h"
#include "network_related_fns.h"
#include "camera_app.h"
#include "flash_files.h"
#include "mt9d111.h"
extern volatile unsigned char g_CaptureImage;
char Token[100]="";
char Value[100]="";



int32_t initNetwork(signed char *ssid, SlSecParams_t *keyParams);
extern int32_t NWP_SwitchOn();
extern int32_t NWP_SwitchOff();
extern int32_t CaptureandSavePreviewImage();

//!    ######################### list of SNTP servers ##################################
//!    ##
//!    ##          hostname         |        IP       |       location
//!    ## -----------------------------------------------------------------------------
//!    ##   nist1-nj2.ustiming.org  | 165.193.126.229 |  Weehawken, NJ
//!    ##   nist1-pa.ustiming.org   | 206.246.122.250 |  Hatfield, PA
//!    ##   time-a.nist.gov         | 129.6.15.28     |  NIST, Gaithersburg, Maryland
//!    ##   time-b.nist.gov         | 129.6.15.29     |  NIST, Gaithersburg, Maryland
//!    ##   time-c.nist.gov         | 129.6.15.30     |  NIST, Gaithersburg, Maryland
//!    ##   ntp-nist.ldsbc.edu      | 198.60.73.8     |  LDSBC, Salt Lake City, Utah
//!    ##   nist1-macon.macon.ga.us | 98.175.203.200  |  Macon, Georgia
//!
//!    ##   For more SNTP server link visit 'http://tf.nist.gov/tf-cgi/servers.cgi'
//!    ###################################################################################
const char g_acSNTPserver[30] = "nist1-macon.macon.ga.us"; //Add any one of the above servers

// Tuesday is the 1st day in 2013 - the relative year
const char g_acDaysOfWeek2013[7][3] = {{"Tue"},
                                    {"Wed"},
                                    {"Thu"},
                                    {"Fri"},
                                    {"Sat"},
                                    {"Sun"},
                                    {"Mon"}};

const char g_acMonthOfYear[12][3] = {{"Jan"},
                                  {"Feb"},
                                  {"Mar"},
                                  {"Apr"},
                                  {"May"},
                                  {"Jun"},
                                  {"Jul"},
                                  {"Aug"},
                                  {"Sep"},
                                  {"Oct"},
                                  {"Nov"},
                                  {"Dec"}};

const char g_acNumOfDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

const char g_acDigits[] = "0123456789";

struct
{
    unsigned long ulDestinationIP;
    int iSockID;
    unsigned long ulElapsedSec;
    short isGeneralVar;
    unsigned long ulGeneralVar;
    unsigned long ulGeneralVar1;
    char acTimeStore[30];
    char *pcCCPtr;
    unsigned short uisCCLen;
}g_TimeData;


//****************************************************************************
//
//!    \brief This function initializes the application variables
//!
//!    \param[in]  None
//!
//!    \return     0 on success, negative error-code on error
//
//****************************************************************************
void InitializeUserConfigVariables()
{
	g_ulSimplelinkStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));

    g_ucProfileAdded = 0;
    g_ucConnectedToConfAP = 0;
	g_ucProvisioningDone = 0;
	g_PhoneConnected_ToCC3200AP_flag = 0;

	g_ucAngle90 = BUTTON_NOT_PRESSED;
	g_ucAngle40 = BUTTON_NOT_PRESSED;
	g_ucExitButton = BUTTON_NOT_PRESSED;
	g_ucConfig= BUTTON_NOT_PRESSED;
	g_ucCalibration= BUTTON_NOT_PRESSED;
	g_ucPushButtonPressedTwice = BUTTON_NOT_PRESSED;
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

    UART_PRINT("b sl_start()1\n\r");	//Tag:Rm
    //lMode = sl_Start(0, 0, 0);
    //ASSERT_ON_ERROR(lMode);
    lRetVal = NWP_SwitchOn();
    ASSERT_ON_ERROR(lRetVal);
    UART_PRINT("a sl_start()1\n\r");	//Tag:Rm

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulSimplelinkStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = NWP_SwitchOff();
        ASSERT_ON_ERROR(lRetVal);

        //lRetVal = sl_Start(0, 0, 0);
        lRetVal = NWP_SwitchOn();
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

//    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
//    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
//    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
//    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
//    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
//    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
//    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    //lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
    //                            SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                    SL_CONNECTION_POLICY(1, 0, 0, 0, 0), NULL, 0);
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
        while(IS_CONNECTED(g_ulSimplelinkStatus))
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

    lRetVal = NWP_SwitchOff();
    ASSERT_ON_ERROR(lRetVal);


    InitializeUserConfigVariables();

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
    //lRetVal = NWP_SwitchOff();
    lRetVal = sl_Stop(0xFE);
    CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);

    //return sl_Start(NULL,NULL,NULL);
    return NWP_SwitchOn();
}

long GetSNTPTime(SlDateTime_t *dateTime, unsigned char ucGmtDiffHr, unsigned char ucGmtDiffMins)
{

	SlSockAddr_t sAddr;
	SlSockAddrIn_t sLocalAddr;

/*
                            NTP Packet Header:


       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          Root  Delay                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root  Dispersion                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Reference Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Reference Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Originate Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Receive Timestamp (64)                     |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Transmit Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                 Key Identifier (optional) (32)                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                                                                |
      |                 Message Digest (optional) (128)                |
      |                                                                |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/
    char cDataBuf[48];
    long lRetVal = 0;
    int iAddrSize;
    //
    // Send a query ? to the NTP server to get the NTP time
    //
    memset(cDataBuf, 0, sizeof(cDataBuf));
    cDataBuf[0] = '\x1b';

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 0x00;
    sAddr.sa_data[1] = 0x7B;    // UDP port number for NTP is 123
    sAddr.sa_data[2] = (char)((g_TimeData.ulDestinationIP>>24)&0xff);
    sAddr.sa_data[3] = (char)((g_TimeData.ulDestinationIP>>16)&0xff);
    sAddr.sa_data[4] = (char)((g_TimeData.ulDestinationIP>>8)&0xff);
    sAddr.sa_data[5] = (char)(g_TimeData.ulDestinationIP&0xff);

    lRetVal = sl_SendTo(g_TimeData.iSockID,
                     cDataBuf,
                     sizeof(cDataBuf), 0,
                     &sAddr, sizeof(sAddr));
    if (lRetVal != sizeof(cDataBuf))
    {
        // could not send SNTP request
        ASSERT_ON_ERROR(-2050);
    }

    //
    // Wait to receive the NTP time from the server
    //
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = 0;
    sLocalAddr.sin_addr.s_addr = 0;
    if(g_TimeData.ulElapsedSec == 0)
    {
        lRetVal = sl_Bind(g_TimeData.iSockID,
                (SlSockAddr_t *)&sLocalAddr,
                sizeof(SlSockAddrIn_t));
    }

    iAddrSize = sizeof(SlSockAddrIn_t);

    lRetVal = sl_RecvFrom(g_TimeData.iSockID,
                       cDataBuf, sizeof(cDataBuf), 0,
                       (SlSockAddr_t *)&sLocalAddr,
                       (SlSocklen_t*)&iAddrSize);
    ASSERT_ON_ERROR(lRetVal);

    //
    // Confirm that the MODE is 4 --> server
    //
    if ((cDataBuf[0] & 0x7) != 4)    // expect only server response
    {
         ASSERT_ON_ERROR(-2050);  // MODE is not server, abort
    }
    else
    {
        unsigned char iIndex;

        //
        // Getting the data from the Transmit Timestamp (seconds) field
        // This is the time at which the reply departed the
        // server for the client
        //
        g_TimeData.ulElapsedSec = cDataBuf[40];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[41];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[42];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[43];

        //
        // seconds are relative to 0h on 1 January 1900
        //
        g_TimeData.ulElapsedSec -= TIME2013;

        //
        // in order to correct the timezone
        //
        g_TimeData.ulElapsedSec += (ucGmtDiffHr * SEC_IN_HOUR);
        g_TimeData.ulElapsedSec += (ucGmtDiffMins * SEC_IN_MIN);

        g_TimeData.pcCCPtr = &g_TimeData.acTimeStore[0];

        //
        // day, number of days since beginning of 2013
        //
        g_TimeData.isGeneralVar = g_TimeData.ulElapsedSec/SEC_IN_DAY;
        memcpy(g_TimeData.pcCCPtr,
               g_acDaysOfWeek2013[g_TimeData.isGeneralVar%7], 3);
        g_TimeData.pcCCPtr += 3;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // month
        //
        g_TimeData.isGeneralVar %= 365;
        for (iIndex = 0; iIndex < 12; iIndex++)
        {
        	g_TimeData.isGeneralVar -= g_acNumOfDaysPerMonth[iIndex];
            if (g_TimeData.isGeneralVar < 0)
                    break;
        }
        if(iIndex == 12)
        {
            iIndex = 0;
        }


        	dateTime->sl_tm_mon = iIndex + 1;



        memcpy(g_TimeData.pcCCPtr, g_acMonthOfYear[iIndex], 3);
        g_TimeData.pcCCPtr += 3;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // date
        // restore the day in current month
        //
        g_TimeData.isGeneralVar += g_acNumOfDaysPerMonth[iIndex];
        g_TimeData.uisCCLen = intToASCII(g_TimeData.isGeneralVar + 1,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = '\x20';

        dateTime->sl_tm_day = g_TimeData.isGeneralVar + 1;
        //
        // time
        //
        g_TimeData.ulGeneralVar = g_TimeData.ulElapsedSec%SEC_IN_DAY;

        // number of seconds per hour
        g_TimeData.ulGeneralVar1 = g_TimeData.ulGeneralVar%SEC_IN_HOUR;

        // number of hours
        g_TimeData.ulGeneralVar /= SEC_IN_HOUR;

        dateTime->sl_tm_hour = g_TimeData.ulGeneralVar;

        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = ':';

        // number of minutes per hour
        g_TimeData.ulGeneralVar = g_TimeData.ulGeneralVar1/SEC_IN_MIN;

        // number of seconds per minute
        g_TimeData.ulGeneralVar1 %= SEC_IN_MIN;
        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = ':';
        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar1,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // year
        // number of days since beginning of 2013
        //
        g_TimeData.ulGeneralVar = g_TimeData.ulElapsedSec/SEC_IN_DAY;
        g_TimeData.ulGeneralVar /= 365;
        g_TimeData.uisCCLen = intToASCII(YEAR2013 + g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;

        dateTime->sl_tm_year = YEAR2013 + g_TimeData.ulGeneralVar;

        *g_TimeData.pcCCPtr++ = '\0';

        UART_PRINT("response from server: ");
        UART_PRINT((char *)g_acSNTPserver);
        UART_PRINT(":");
        UART_PRINT(g_TimeData.acTimeStore);
    }
    return SUCCESS;
}

long Network_IF_GetHostIP( char* pcHostName,unsigned long * pDestinationIP )
{

	long lStatus=-1;
	lStatus = sl_NetAppDnsGetHostByName((signed char *) pcHostName,
                                            strlen(pcHostName),
                                            pDestinationIP, SL_AF_INET);
    ASSERT_ON_ERROR(lStatus);

    return lStatus;

//    UART_PRINT("Get Host IP succeeded.\n\rHost: %s IP: %d.%d.%d.%d \n\r\n\r",
//                    pcHostName, SL_IPV4_BYTE(*pDestinationIP,3),
//                    SL_IPV4_BYTE(*pDestinationIP,2),
//                    SL_IPV4_BYTE(*pDestinationIP,1),
//                    SL_IPV4_BYTE(*pDestinationIP,0));

}

int32_t GetTimeNTP(SlDateTime_t *dateTime)
{
    int iSocketDesc;
    long lRetVal = -1;
    int count=0;

    //
    // Create UDP socket
    //
    iSocketDesc = sl_Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(iSocketDesc < 0)
    {
        ERR_PRINT(iSocketDesc);
    }

    //
    // Get the NTP server host IP address using the DNS lookup
    //
    lRetVal = Network_IF_GetHostIP((char*)g_acSNTPserver, \
                                    &g_TimeData.ulDestinationIP);

    if( lRetVal >= 0)
    {

        struct SlTimeval_t timeVal;
        timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
        timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
        lRetVal = sl_SetSockOpt(g_TimeData.iSockID,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
                        (unsigned char*)&timeVal, sizeof(timeVal));

        ASSERT_ON_ERROR(lRetVal);

        do
        {
            //
            // Get the NTP time and display the time
            //
            lRetVal = GetSNTPTime(dateTime,GMT_DIFF_TIME_HRS, GMT_DIFF_TIME_MINS);
            if(lRetVal == 0)
            {
                UART_PRINT("\nTime received from NTP server\n\r");
                break;
            }

            //
            // Wait a while before resuming
            //
            MAP_UtilsDelay(SLEEP_TIME);
            count++;
        }
        while(count < 5);
        ASSERT_ON_ERROR(lRetVal);
    }
    else
    {
        UART_PRINT("DNS lookup failed. \n\r");
    }

    //
    // Close the socket
    //
    close(iSocketDesc);
    return SUCCESS;
}


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
	UART_PRINT("b sl_start\n\r");
	//lRetVal = sl_Start(0, 0, 0);
	lRetVal = NWP_SwitchOn();
	UART_PRINT("a sl_start\n\r");
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = ReadFile_FromFlash(ucWifiConfigFileData,
									(uint8_t*)USER_CONFIGS_FILENAME,
									WIFI_DATA_SIZE, WIFI_DATA_OFFSET);

	pucWifiConfigFileData = &ucWifiConfigFileData[0];
	ASSERT_ON_ERROR(lRetVal);

	strcpy((char*)ssid, (const char*)ucWifiConfigFileData);
	pucWifiConfigFileData += strlen((const char*)pucWifiConfigFileData)+1;
	strcpy((char*)ucWifiSecType, (const char*)pucWifiConfigFileData);
	pucWifiConfigFileData += strlen((const char*)pucWifiConfigFileData)+1;
	strcpy((char*)ucWifiPassword, (const char*)pucWifiConfigFileData);

	keyParams.Type = ucWifiSecType[0];
	keyParams.Key = ucWifiPassword;
	keyParams.KeyLen = strlen((char *)ucWifiPassword);

	lRetVal = initNetwork(ssid, &keyParams);
	ASSERT_ON_ERROR(lRetVal);

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
	ASSERT_ON_ERROR(status);

	sl_WlanDisconnect();

	status = sl_WlanSetMode(ROLE_STA);
	ASSERT_ON_ERROR(status);

	//UART_PRINT("\r\n[QuickStart] Network init\r\n");

//	ssid = "Soliton";
//	keyParams->Key = "37203922bb";
//	keyParams->KeyLen = sizeof("37203922bb");
//	keyParams->Type = SL_SEC_TYPE_WPA_WPA2;
//	ssid = "Camera";
//	keyParams->Key = "abcdef1234";
//	keyParams->KeyLen = sizeof("abcdef1234");
//	keyParams->Type = SL_SEC_TYPE_WPA_WPA2;
//	ssid = "kris fire";
//	keyParams->Key = "cfuclcxjfdi";
//	keyParams->KeyLen = sizeof("cfuclcxjfdi");
//	keyParams->Type = SL_SEC_TYPE_WPA_WPA2;

	UART_PRINT("WiFi\n");
	UART_PRINT("SSID:%s\nPassword:%s\nSecurity Type:%x\n", ssid, keyParams->Key, keyParams->Type);
	status = sl_WlanConnect(ssid, strlen((char *)ssid), NULL, keyParams, NULL);
	ASSERT_ON_ERROR(status);

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
            SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_CONNECTION);

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

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s ,"
            		"BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      g_ucConnectionSSID,g_ucConnectionBSSID[0],
                      g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                      g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                      g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_IP_AQUIRED);
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));

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
                UART_PRINT("Reason Code: %x\n\r", pEventData->reason_code);

                if(SL_INVALID_INFORMATION_ELEMENT == pEventData->reason_code)
                {
                	UART_PRINT("Reason: SL_INVALID_INFORMATION_ELEMENT\n\r");
                }
                else if(SL_MESSAGE_INTEGRITY_CODE_MIC_FAILURE == pEventData->reason_code)
                {
					UART_PRINT("Reason: SL_MESSAGE_INTEGRITY_CODE_MIC_FAILURE\n\r");
				}
                else if(SL_FOUR_WAY_HANDSHAKE_TIMEOUT == pEventData->reason_code)
				{
					UART_PRINT("Reason: SL_FOUR_WAY_HANDSHAKE_TIMEOUT\n\r");
				}
//                else if (SL_ROAMING_TRIGGER_BSS_LOSS == pEventData->reason_code)
//				{
//					UART_PRINT("Reason: SL_ROAMING_TRIGGER_BSS_LOSS\n\r");
//				}
                // cc3200 gets stuck after this. So, a workaround is to reset
                //and begin again
                PRCMSOCReset();
                /*sl_Stop(0);
                UART_PRINT("NWP stop done");
                sl_Start(0,0,0);
                UART_PRINT("NWP restart done");*/
            }
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
        	UART_PRINT("[WLAN EVENT] Mobile Station connected to CC3200-AP\n\r");
        	g_PhoneConnected_ToCC3200AP_flag = 1;
        	LED_Blink_2(.5,.5,BLINK_FOREVER);
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

            SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_IP_AQUIRED);

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
        case SL_NETAPP_IP_LEASED_EVENT:
        {
        	UART_PRINT("[NETAPP EVENT] CC3200-AP has leased IP to Mobile Station\n\r");
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

    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status )
            {
                case SL_ECLOSE:
                    UART_PRINT("\n[SOCK ERROR] - close socket (%d) operation "
                    "failed to transmit all queued packets\n",
					pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default:
                    UART_PRINT("\n[SOCK ERROR] - TX FAILED : socket %d , reason"
                        "(%d) \n",
						pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
                    if(SL_ENOTCONN == pSock->socketAsyncEvent.SockTxFailData.status)
                    {
                    	UART_PRINT("-107 = SL_ENOTCONN = "
                    			"Transport endpoint is not connected\n\r");
                    }
            }
            break;

        default:
            UART_PRINT("\n[SOCK EVENT] "
            			"- Unexpected Event [%x0x]\n",pSock->Event);
    }
//    UART_PRINT("[SOCK EVENT/ERROR]:\n Event: %x\n Socket: %x\n EventStatus: "
//    				"%d\n AsynchEventValue: %x\n AsynchEventType: %x\n\r\n\r",
//    				pSock->Event, pSock->EventData.sd, pSock->EventData.status,
//    				pSock->EventData.socketAsyncEvent.val,
//    				pSock->EventData.socketAsyncEvent.type);
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
	uint8_t G1Gain,RGain,BGain,G2Gain;

    switch (pSlHttpServerEvent->Event)
    {
        case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:
        {
        	UART_PRINT("Received GET Token");
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [0], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                if(g_ucConnectedToConfAP == 1)
                {
                    // Important - Connection Status
                    memcpy(pSlHttpServerResponse->ResponseData.token_value.data, "TRUE",strlen("TRUE"));
                    pSlHttpServerResponse->ResponseData.token_value.len = strlen("TRUE");
                }
                else
                {
                    // Important - Connection Status
                    memcpy(pSlHttpServerResponse->ResponseData.token_value.data, "FALSE",strlen("FALSE"));
                    pSlHttpServerResponse->ResponseData.token_value.len = strlen("FALSE");
                }
            }

            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [1], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                // Important - Token value len should be < MAX_TOKEN_VALUE_LEN
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, g_NetEntries[0].ssid,g_NetEntries[0].ssid_len);
                pSlHttpServerResponse->ResponseData.token_value.len = g_NetEntries[0].ssid_len;
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [2], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                // Important - Token value len should be < MAX_TOKEN_VALUE_LEN
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, g_NetEntries[1].ssid,g_NetEntries[1].ssid_len);
                pSlHttpServerResponse->ResponseData.token_value.len = g_NetEntries[1].ssid_len;
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [3], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                // Important - Token value len should be < MAX_TOKEN_VALUE_LEN
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, g_NetEntries[2].ssid,g_NetEntries[2].ssid_len);
                pSlHttpServerResponse->ResponseData.token_value.len = g_NetEntries[2].ssid_len;
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [4], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                // Important - Token value len should be < MAX_TOKEN_VALUE_LEN
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, g_NetEntries[3].ssid,g_NetEntries[3].ssid_len);
                pSlHttpServerResponse->ResponseData.token_value.len = g_NetEntries[3].ssid_len;
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, g_token_get [5], pSlHttpServerEvent->EventData.httpTokenName.len))
            {
                // Important - Token value len should be < MAX_TOKEN_VALUE_LEN
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, g_NetEntries[4].ssid,g_NetEntries[4].ssid_len);
                pSlHttpServerResponse->ResponseData.token_value.len = g_NetEntries[4].ssid_len;
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, "Button90", pSlHttpServerEvent->EventData.httpTokenName.len))
            {
            	g_ucAngle90 = BUTTON_PRESSED;
            	while(g_ucAngle90 != ANGLE_VALUE_COLLECTED);
            	g_ucAngle90 = BUTTON_NOT_PRESSED;
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, "Angle90 Collected",strlen("Angle90 Collected"));
                pSlHttpServerResponse->ResponseData.token_value.len = strlen("Angle90 Collected");
            }
            if(0== memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, "Button40", pSlHttpServerEvent->EventData.httpTokenName.len))
            {
            	g_ucAngle90 = BUTTON_PRESSED;
            	while(g_ucAngle90 != ANGLE_VALUE_COLLECTED);
            	g_ucAngle90 = BUTTON_NOT_PRESSED;
                memcpy(pSlHttpServerResponse->ResponseData.token_value.data, "Angle40 Collected",strlen("Angle40 Collected"));
                pSlHttpServerResponse->ResponseData.token_value.len = strlen("Angle40 Collected");
            }

        }
        break;

        case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
        {

        	UART_PRINT("\nReceived POST Token\n\r");
        	uint16_t Token_Len = pSlHttpServerEvent->EventData.httpPostData.token_name.len;
        	uint16_t Value_Len =  pSlHttpServerEvent->EventData.httpPostData.token_value.len;
        	memcpy(Value,  pSlHttpServerEvent->EventData.httpPostData.token_value.data,Value_Len);
        	memcpy(Token,  pSlHttpServerEvent->EventData.httpPostData.token_name.data,Token_Len);
        	Token[Token_Len] = '\0';
        	Value[Value_Len] = '\0';
        	UART_PRINT("Token : %s\n\r",Token);
        	UART_PRINT("Value : %s\n\r",Value);

        	//angle90 , angle40
        	if(0 == memcmp(Token, TOK_ANGLE, Token_Len))
			{
        		if(0 == memcmp(Value, "Angle90", Value_Len))
        		{
        			g_ucAngle90 = BUTTON_PRESSED;

        		}
        		else if(0 == memcmp(Value, "Angle40", Value_Len))
        		{
        			g_ucAngle40 = BUTTON_PRESSED;
        		}
			}
           	else if(0 == memcmp(Token, TOK_CALIB, Token_Len))
			{
				if(0 == memcmp(Value, "Calibrate", Value_Len))
				{
					g_ucCalibration = BUTTON_PRESSED;
				}
			}
        	else if(0 == memcmp(Token, TOK_SSID, Token_Len))		//__SL_P_USD
        	{
        		memcpy(g_cWlanSSID,  Value, Value_Len);
        		g_cWlanSSID[pSlHttpServerEvent->EventData.httpPostData.token_value.len] = 0;
        	}
        	//Security Type
        	else if(0 == memcmp(Token, TOK_KEYTYPE, Token_Len))		//__SL_P_USE
        	{
				if(Value[0] == '0')
				{
					g_SecParams.Type =  SL_SEC_TYPE_OPEN;//SL_SEC_TYPE_OPEN
				}
				else if(Value[0] == '1')
				{
					g_SecParams.Type =  SL_SEC_TYPE_WEP;//SL_SEC_TYPE_WEP
				}
				else if(Value[0]== '2')
				{
					g_SecParams.Type =  SL_SEC_TYPE_WPA;//SL_SEC_TYPE_WPA
				}
				else
				{
					g_SecParams.Type =  SL_SEC_TYPE_OPEN;//SL_SEC_TYPE_OPEN
				}
				g_cWlanSecurityType[0] = g_SecParams.Type;
				g_cWlanSecurityType[1] = 0;
        	}

        	//Security Key
        	else if(0 == memcmp(Token, TOK_KEY, Token_Len))		//__SL_P_USF
        	{
				memcpy(g_cWlanSecurityKey,Value, Value_Len);
				g_cWlanSecurityKey[Value_Len] = 0;
				g_SecParams.Key = g_cWlanSecurityKey;
				g_SecParams.KeyLen = Value_Len;
			}
        	else if(0 == memcmp(Token, TOK_CONFIG_WIFI, Token_Len))		//__SL_P_US0
        	{
        		g_ucConfig = BUTTON_PRESSED;
            }
        	else if(0 == memcmp(Token, TOK_EXIT, Token_Len))
        	{
        		if(0 == memcmp(Value, "Exit", Value_Len))
				{
        			g_ucExitButton = BUTTON_PRESSED;
				}
        	}
        	else if(0 == memcmp(Token, TOK_ACTION1, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value,"Integ",5))
        		{
        			//Copy shutter Width
        			uint16_t ShutterWidth = atoi(&Value[6]);
        			SetShutterWidth(ShutterWidth);
        		}

            }
        	else if(0 == memcmp(Token, TOK_ACTION2, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value,"AGain",5))
        		{
        			//Copy Analog Gain
        			G1Gain = atoi(&Value[6]);
        			RGain = atoi(&Value[9]);
        			BGain = atoi(&Value[12]);
        			G2Gain = atoi(&Value[15]);
        			SetAnalogGain(G1Gain,RGain,BGain,G2Gain);
        		}
        		if(0 == memcmp(Value,"DGain",5))
        		{
        			//Copy Digital Gain

					//Copy Analog Gain
					G1Gain = atoi(&Value[6]);
					RGain = atoi(&Value[9]);
					BGain = atoi(&Value[12]);
					G2Gain = atoi(&Value[15]);
					SetDigitalGain(G1Gain,RGain,BGain,G2Gain);


        		}
        		if(0 == memcmp(Value,"IGain",5))
        		{
        			//Copy Initial Gain
					//Copy Analog Gain
					G1Gain = atoi(&Value[6]);
					RGain = atoi(&Value[9]);
					BGain = atoi(&Value[12]);
					G2Gain = atoi(&Value[15]);
					SetInitialGain(G1Gain,RGain,BGain,G2Gain);
        		}

            }
        	else if(0 == memcmp(Token, TOK_ACTION3, Token_Len))		//__SL_P_US0
        	{

        		if(0 == memcmp(Value, "Restart", Value_Len))
				{
        			g_ucActionButton = BUTTON_PRESSED;
					g_ucAction = CAM_RESTART_CAPTURE;
				}
            }
        	else if(0 == memcmp(Token, TOK_ACTION4, Token_Len))		//__SL_P_US0
        	{

            }
        	else if(0 == memcmp(Token, TOK_PREVIEW, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "Start", Value_Len))
				{
        			g_ucPreviewStart = BUTTON_PRESSED;
				}
        		else if(0 == memcmp(Value, "Stop", Value_Len))
				{
        			g_ucPreviewStop = BUTTON_PRESSED;
				}
            }
        	else if(0 == memcmp(Token, TOK_AWB, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "AWBON", Value_Len))
				{
        			g_ucAWBOn = BUTTON_PRESSED;
				}
        		else if(0 == memcmp(Value, "AWBOFF", Value_Len))
				{
        			g_ucAWBOff = BUTTON_PRESSED;
				}
            }
        	else if(0 == memcmp(Token, TOK_SAVE, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "SAVE", Value_Len))
				{
        			g_ucSAVE = BUTTON_PRESSED;
				}

            }

        }
        break;

      default:
          break;
    }
}
