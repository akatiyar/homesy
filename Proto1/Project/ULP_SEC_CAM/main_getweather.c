//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Get Weather
// Application Overview - Get Weather application connects to "openweathermap.org"
//                        server, requests for weather details of the specified
//                        city, process data and displays it on the Hyperterminal.
//
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_Info_Center_Get_Weather_Application
// or
// docs\examples\CC32xx_Info_Center_Get_Weather_Application.pdf
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup get_weather
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// simplelink includes
#include "device.h"

// driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"
#include "pinmux.h"

//Free_rtos/ti-rtos includes
#include "osi.h"

// common interface includes
#ifndef NOTERM
#include "uartA1_if.h"
#endif
#include "gpio_if.h"
#include "timer_if.h"
#include "network_if.h"
#include "udma_if.h"
#include "common.h"

// HTTP Client lib
#include <http/client/httpcli.h>
#include <http/client/common.h>

//Tag:OTA
//#include "flc_api.h"
#include "ota_api.h"

//****************************************************************************
//                          LOCAL DEFINES                                   
//****************************************************************************
#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME                "Get Weather"

#define SLEEP_TIME              8000000
#define SUCCESS                 0
#define OSI_STACK_SIZE          3000

#define PREFIX_BUFFER "/data/2.5/weather?q="
#define POST_BUFFER "&mode=xml&units=imperial"

#define HOST_NAME       "api.openweathermap.org"
#define HOST_PORT       (80)

//Tag:OTA
#define OTA_SERVER_NAME                 "api.dropbox.com"
#define OTA_SERVER_IP_ADDRESS           0x00000000
#define OTA_SERVER_SECURED              1
#define OTA_SERVER_REST_UPDATE_CHK      "/1/metadata/auto/" // returns files/folder list
#define OTA_SERVER_REST_RSRC_METADATA   "/1/media/auto"     // returns A url that serves the media directly
#define OTA_SERVER_REST_HDR             "Authorization: Bearer "
//#define OTA_SERVER_REST_HDR_VAL         "gpmUoDDvCoAAAAAAAAAABd0vZXQLq5Br36HLFYl45ssi4ovc58y9G4xEET39CjKc"	//ULPC_OTA
#define OTA_SERVER_REST_HDR_VAL         "gpmUoDDvCoAAAAAAAAAABz3goxouL7gTfYq9KZZMcNYMES8mwG_cihHC-lbiWd8f"		//TISDK_OTA, Get Waether
#define LOG_SERVER_NAME                 "api-content.dropbox.com"
#define OTA_SERVER_REST_FILES_PUT       "/1/files_put/auto/"
//#define OTA_VENDOR_STRING               "Vid00_Pid00_Ver00"
//#define OTA_VENDOR_STRING               "TI_CC3200_WTHR1"
#define OTA_VENDOR_STRING               "TI_CC3200_WTHR_0"
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SERVER_GET_WEATHER_FAILED = -0x7D0,
    WRONG_CITY_NAME = SERVER_GET_WEATHER_FAILED - 1,
    NO_WEATHER_DATA = WRONG_CITY_NAME - 1,
    DNS_LOOPUP_FAILED = WRONG_CITY_NAME  -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

unsigned long g_ulTimerInts;   //  Variable used in Timer Interrupt Handler
SlSecParams_t SecurityParams = {0};  // AP Security Parameters

char acSendBuff[512];	// Buffer to send data
char acRecvbuff[1460];  // Buffer to receive data

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//Tag:OTA
static OtaOptServerInfo_t g_otaOptServerInfo;
static void *pvOtaApp;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//****************************************************************************
static long GetWeather(HTTPCli_Handle cli, int iSockID, char *pcCityName);
void GetWeatherTask(void *pvParameters);
static void BoardInit();
static void DisplayBanner(char * AppName);
static int HandleXMLData(char *acRecvbuff);
static int FlushHTTPResponse(HTTPCli_Handle cli);

//Tag:OTA
//****************************************************************************
//
//! Sets the OTA server info and vendor ID
//!
//! \param pvOtaApp pointer to OtaApp handler
//! \param ucVendorStr vendor string
//! \param pfnOTACallBack is  pointer to callback function
//!
//! This function sets the OTA server info and vendor ID.
//!
//! \return None.
//
//****************************************************************************
int OTAServerInfoSet(void **pvOtaApp, char *vendorStr)
{
	unsigned char macAddressLen = SL_MAC_ADDR_LEN;

    //
    // Set OTA server info
    //
    g_otaOptServerInfo.ip_address = OTA_SERVER_IP_ADDRESS;
    g_otaOptServerInfo.secured_connection = OTA_SERVER_SECURED;
    strcpy((char *)g_otaOptServerInfo.server_domain, OTA_SERVER_NAME);
    strcpy((char *)g_otaOptServerInfo.rest_update_chk, OTA_SERVER_REST_UPDATE_CHK);
    strcpy((char *)g_otaOptServerInfo.rest_rsrc_metadata, OTA_SERVER_REST_RSRC_METADATA);
    strcpy((char *)g_otaOptServerInfo.rest_hdr, OTA_SERVER_REST_HDR);
    strcpy((char *)g_otaOptServerInfo.rest_hdr_val, OTA_SERVER_REST_HDR_VAL);
    strcpy((char *)g_otaOptServerInfo.log_server_name, LOG_SERVER_NAME);
    strcpy((char *)g_otaOptServerInfo.rest_files_put, OTA_SERVER_REST_FILES_PUT);
    sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &macAddressLen, (_u8*)g_otaOptServerInfo.log_mac_address);
    //NetMACAddressGet(g_otaOptServerInfo.log_mac_address);

    //
    // Set OTA server Info
    //
    sl_extLib_OtaSet(*pvOtaApp, EXTLIB_OTA_SET_OPT_SERVER_INFO,
                     sizeof(g_otaOptServerInfo), (_u8 *)&g_otaOptServerInfo);

    //
    // Set vendor ID.
    //
    sl_extLib_OtaSet(*pvOtaApp, EXTLIB_OTA_SET_OPT_VENDOR_ID, strlen(vendorStr),
                     (_u8 *)vendorStr);

    //
    // Return ok status
    //
    return RUN_STAT_OK;
}
//****************************************************************************
//
//! Reboot the MCU by requesting hibernate for a short duration
//!
//! \return None
//
//****************************************************************************
static void RebootMCU()
{

  //
  // Configure hibernate RTC wakeup
  //
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

  //
  // Delay loop
  //
  MAP_UtilsDelay(8000000);

  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet(330);

  //
  // Request hibernate
  //
  PRCMHibernateEnter();

  //
  // Control should never reach here
  //
  while(1)
  {

  }
}


//*****************************************************************************
//
//! Periodic Timer Interrupt Handler
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
TimerPeriodicIntHandler(void)
{
    unsigned long ulInts;

    //
    // Clear all pending interrupts from the timer we are
    // currently using.
    //
    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    //
    // Increment our interrupt counter.
    //
    g_ulTimerInts++;
    if(!(g_ulTimerInts & 0x1))
    {
        //
        // Off Led
        //
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    }
    else
    {
        //
        // On Led
        //
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    }
}

//****************************************************************************
//
//! Function to configure and start timer to blink the LED while device is
//! trying to connect to an AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerConfigNStart()
{
    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    Timer_IF_Init(PRCM_TIMERA0,TIMERA0_BASE,TIMER_CFG_PERIODIC,TIMER_A,0);
    Timer_IF_IntSetup(TIMERA0_BASE,TIMER_A,TimerPeriodicIntHandler);
    Timer_IF_Start(TIMERA0_BASE,TIMER_A,100);  // Time value is in mSec
}

//****************************************************************************
//
//! Disable the LED blinking Timer as Device is connected to AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerDeinitStop()
{
    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    Timer_IF_Stop(TIMERA0_BASE,TIMER_A);
    Timer_IF_DeInit(TIMERA0_BASE,TIMER_A);
}

//****************************************************************************
//
//! Parsing XML data to dig wetaher information
//!
//! \param[in] acRecvbuff - XML Data
//!
//! return 0 on success else -ve
//
//****************************************************************************
static int HandleXMLData(char *acRecvbuff)
{
    char *pcIndxPtr;
    char *pcEndPtr;

    //
    // Get city name
    //
    pcIndxPtr = strstr(acRecvbuff, "name=");
    if(pcIndxPtr == NULL)
    {
        ASSERT_ON_ERROR(WRONG_CITY_NAME);
    }

    DBG_PRINT("\n\r****************************** \n\r\n\r");
    DBG_PRINT("City: ");
    if( NULL != pcIndxPtr )
    {
        pcIndxPtr = pcIndxPtr + strlen("name=") + 1;
        pcEndPtr = strstr(pcIndxPtr, "\">");
        if( NULL != pcEndPtr )
        {
           *pcEndPtr = 0;
        }
        DBG_PRINT("%s\n\r",pcIndxPtr);
    }
    else
    {
        DBG_PRINT("N/A\n\r");
        return NO_WEATHER_DATA;
    }

    //
    // Get temperature value
    //
    pcIndxPtr = strstr(pcEndPtr+1, "temperature value");
    DBG_PRINT("Temperature: ");
    if( NULL != pcIndxPtr )
    {
        pcIndxPtr = pcIndxPtr + strlen("temperature value") + 2;
        pcEndPtr = strstr(pcIndxPtr, "\" ");
        if( NULL != pcEndPtr )
        {
           *pcEndPtr = 0;
        }
        DBG_PRINT("%s\n\r",pcIndxPtr);
    }
    else
    {
        DBG_PRINT("N/A\n\r");
        return NO_WEATHER_DATA;
    }
    
    
    //
    // Get weather condition
    //
    pcIndxPtr = strstr(pcEndPtr+1, "weather number");
    DBG_PRINT("Weather Condition: ");
    if( NULL != pcIndxPtr )
    {
        pcIndxPtr = pcIndxPtr + strlen("weather number") + 14;
        pcEndPtr = strstr(pcIndxPtr, "\" ");
        if( NULL != pcEndPtr )
        {
           *pcEndPtr = 0;
        }
        DBG_PRINT("%s\n\r",pcIndxPtr);
    }
    else
    {
        DBG_PRINT("N/A\n\r");
        return NO_WEATHER_DATA;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! This function flush received HTTP response
//!
//! \param[in] cli - HTTP client object
//!
//! return 0 on success else -ve
//
//****************************************************************************
static int FlushHTTPResponse(HTTPCli_Handle cli)
{
    const char *ids[2] = {
                            HTTPCli_FIELD_NAME_CONNECTION, /* App will get connection header value. all others will skip by lib */
                            NULL
                         };
    char buf[128];
    int id;
    int len = 1;
    bool moreFlag = 0;
    char ** prevRespFilelds = NULL;


    prevRespFilelds = HTTPCli_setResponseFields(cli, ids);

    //
    // Read response headers
    //
    while ((id = HTTPCli_getResponseField(cli, buf, sizeof(buf), &moreFlag))
            != HTTPCli_FIELD_ID_END)
    {

        if(id == 0)
        {
            if(!strncmp(buf, "close", sizeof("close")))
            {
                UART_PRINT("Connection terminated by server\n\r");
            }
        }

    }

    HTTPCli_setResponseFields(cli, (const char **)prevRespFilelds);

    while(1)
    {
        len = HTTPCli_readResponseBody(cli, buf, sizeof(buf) - 1, &moreFlag);
        ASSERT_ON_ERROR(len);

        if ((len - 2) >= 0 && buf[len - 2] == '\r' && buf [len - 1] == '\n')
        {

        }

        if(!moreFlag)
        {
            break;
        }
    }
    return SUCCESS;
}

//*****************************************************************************
//
//! GetWeather
//!
//! \brief  Obtaining the weather info for the specified city from the server
//!
//! \param  iSockID is the socket ID
//! \param  pcCityName is the pointer to the name of the city
//!
//! \return none.        
//!
//
//*****************************************************************************
static long GetWeather(HTTPCli_Handle cli, int iSockID, char *pcCityName)
{

    char* pcBufLocation;
    long lRetVal = 0;
    int id;
    int len = 1;
    bool moreFlag = 0;
    char ** prevRespFilelds = NULL;
    HTTPCli_Field fields[2] = {
                                {HTTPCli_FIELD_NAME_HOST, HOST_NAME},
                                {NULL, NULL},
                              };
    const char *ids[3] = {
                            HTTPCli_FIELD_NAME_CONNECTION,
                            HTTPCli_FIELD_NAME_CONTENT_TYPE,
                            NULL
                         };
    //
    // Set request fields
    //
    HTTPCli_setRequestFields(cli, fields);

    pcBufLocation = acSendBuff;
    strcpy(pcBufLocation, PREFIX_BUFFER);
    pcBufLocation += strlen(PREFIX_BUFFER);
    strcpy(pcBufLocation , pcCityName);
    pcBufLocation += strlen(pcCityName);
    strcpy(pcBufLocation , POST_BUFFER);
    pcBufLocation += strlen(POST_BUFFER);

    //
    // Make HTTP 1.1 GET request
    //
    lRetVal = HTTPCli_sendRequest(cli, HTTPCli_METHOD_GET, acSendBuff, 0);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to send HTTP 1.1 GET request.\n\r");
        return 	SERVER_GET_WEATHER_FAILED;
    }

    //
    // Test getResponseStatus: handle
    //
    lRetVal = HTTPCli_getResponseStatus(cli);
    if (lRetVal != 200)
    {
        UART_PRINT("HTTP Status Code: %d\r\n", lRetVal);
        FlushHTTPResponse(cli);
        return SERVER_GET_WEATHER_FAILED;
    }

    prevRespFilelds = HTTPCli_setResponseFields(cli, ids);

    //
    // Read response headers
    //
    while ((id = HTTPCli_getResponseField(cli, acRecvbuff, sizeof(acRecvbuff), &moreFlag)) 
            != HTTPCli_FIELD_ID_END)
    {

        if(id == 0) // HTTPCli_FIELD_NAME_CONNECTION
        {
            if(!strncmp(acRecvbuff, "close", sizeof("close")))
            {
                UART_PRINT("Connection terminated by server.\n\r");
            }
        }
        else if(id == 1) // HTTPCli_FIELD_NAME_CONTENT_TYPE
        {
            UART_PRINT("Content Type: %s\r\n", acRecvbuff);
        }
    }

    HTTPCli_setResponseFields(cli, (const char **)prevRespFilelds);

    //
    // Read body
    //
    while (1)
    {
        len = HTTPCli_readResponseBody(cli, acRecvbuff, sizeof(acRecvbuff) - 1, &moreFlag);
        if(len < 0)
        {
            return SERVER_GET_WEATHER_FAILED;
        }
        acRecvbuff[len] = 0;

        if ((len - 2) >= 0 && acRecvbuff[len - 2] == '\r'
                && acRecvbuff [len - 1] == '\n')
        {
            break;
        }
        if(!moreFlag)
            break;

        lRetVal = HandleXMLData(acRecvbuff);
        ASSERT_ON_ERROR(lRetVal);
    }


    DBG_PRINT("\n\r****************************** \n\r");
    return SUCCESS;
}


//****************************************************************************
//
//! Task function implementing the getweather functionality
//!
//! \param none
//! 
//! This function  
//!    1. Initializes the required peripherals
//!    2. Initializes network driver and connects to the default AP
//!    3. Creates a TCP socket, gets the server IP address using DNS
//!    4. Gets the weather info for the city specified
//!
//! \return None.
//
//****************************************************************************
void GetWeatherTask(void *pvParameters)
{
    int iSocketDesc;
    int iRetVal;
    char acCityName[32];
    long lRetVal = -1;
    unsigned long ulDestinationIP;
    struct sockaddr_in addr;
    HTTPCli_Struct cli;

    //Tag:OTA
    int SetCommitInt;
    unsigned char ucVendorStr[20];
    long OptionLen;
    unsigned char OptionVal;

    DBG_PRINT("GET_WEATHER: Test Begin\n\r");

    //
    // Configure LED
    //
    GPIO_IF_LedConfigure(LED1|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);    


    //
    // Reset The state of the machine
    //
    Network_IF_ResetMCUStateMachine();

    //
    // Start the driver
    //
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to start SimpleLink Device\n\r");
       return;
    }

    // Switch on Green LED to indicate Simplelink is properly UP
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);

    //Tag:OTA
    //
	// Initialize OTA
	//
	pvOtaApp = sl_extLib_OtaInit(RUN_MODE_NONE_OS | RUN_MODE_BLOCKING,0);
	//
	// Initializa OTA service
	//
	strcpy((char *)ucVendorStr,OTA_VENDOR_STRING);
    OTAServerInfoSet(&pvOtaApp,(char *)ucVendorStr);

    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    LedTimerConfigNStart();

    // Initialize AP security params
    SecurityParams.Key = (signed char *)SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    //
    // Connect to the Access Point
    //
    lRetVal = Network_IF_ConnectAP(SSID_NAME,SecurityParams);
    if(lRetVal < 0)
    {
       UART_PRINT("Connection to an AP failed\n\r");
       LOOP_FOREVER();
    }

    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    LedTimerDeinitStop();

    //
    // Switch ON RED LED to indicate that Device acquired an IP
    //
    GPIO_IF_LedOn(MCU_IP_ALLOC_IND);

    //
    // Get the serverhost IP address using the DNS lookup
    //
    lRetVal = Network_IF_GetHostIP((char*)HOST_NAME, &ulDestinationIP);
    if(lRetVal < 0)
    {
        UART_PRINT("DNS lookup failed. \n\r",lRetVal);
        goto end;
    }

    //Tag:OTA
    //
    //	Check if this image is booted in test mode
    //
    sl_extLib_OtaGet(pvOtaApp, EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT, &OptionLen,(_u8*)&OptionVal);
    UART_PRINT("EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT? %d\n\r", OptionVal);
    if(OptionVal == true)
    {
    	UART_PRINT("OTA: Pending commit and WLAN ok ==> perform commit\n\r");
    	SetCommitInt = OTA_ACTION_IMAGE_COMMITED;
    	sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_COMMIT, sizeof(int),(_u8 *)&SetCommitInt);
    }
    else
    {
    	UART_PRINT("Starting OTA\n\r");
    	lRetVal = 0;

    	while(!lRetVal)
    	{
    		lRetVal = sl_extLib_OtaRun(pvOtaApp);
    	}

    	UART_PRINT("OTA run = %d\n\r", lRetVal);
    	if(lRetVal < 0)
    	{
    		UART_PRINT("OTA:Error with OTA Server\n\r");
    	}
    	else if(lRetVal == RUN_STAT_NO_UPDATES)
    	{
    		UART_PRINT("OTA: RUN_STAT_NO_UPDATES");
    	}
    	else if((lRetVal & RUN_STAT_DOWNLOAD_DONE))
    	{
    		//
    		//	Set OTA File for testing
    		//
    		lRetVal = sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_TEST, sizeof(int), (_u8 *)&SetCommitInt);
    		UART_PRINT("OTA: NEW IMAGE DOWNLOAD COMPLETE\n\r");
    		UART_PRINT("Rebooting...\n\r");
    		RebootMCU();
    	}
    }


    //
    // Set up the input parameters for HTTP Connection
    //
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HOST_PORT);
    addr.sin_addr.s_addr = sl_Htonl(ulDestinationIP);

    //
    // Testing HTTPCli open call: handle, address params only
    //
    HTTPCli_construct(&cli);
    lRetVal = HTTPCli_connect(&cli, (struct sockaddr *)&addr, 0, NULL);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to create instance of HTTP Client.\n\r");
        goto end;
    }    


    while(1)
    {
        //
        // Get the city name over UART to get the weather info
        //
        UART_PRINT("\n\rEnter city name, or QUIT to quit: ");
        iRetVal = GetCmd(acCityName, sizeof(acCityName));
        if(iRetVal > 0)
        {
            if (!strcmp(acCityName,"QUIT") || !strcmp(acCityName,"quit"))
            {
                break;
            }
            else
            {
                //
                // Get the weather info and display the same
                //
                lRetVal = GetWeather(&cli, iSocketDesc, &acCityName[0]);
                if(lRetVal == SERVER_GET_WEATHER_FAILED)
                {
                    UART_PRINT("Server Get Weather failed \n\r");
                    LOOP_FOREVER();
                }
                else if(lRetVal == WRONG_CITY_NAME)
                {
                    UART_PRINT("Wrong input\n\r");

                }
                else if(lRetVal == NO_WEATHER_DATA)
                {
                    UART_PRINT("Weather data not available\n\r");

                }
                else
                {

                }

                //
                // Wait a while before resuming
                //
                MAP_UtilsDelay(SLEEP_TIME);
            }
        }
    }

    HTTPCli_destruct(&cli);
    end:
    //
    // Stop the driver
    //
    lRetVal = Network_IF_DeInitDriver();
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to stop SimpleLink Device\n\r");
       LOOP_FOREVER();
    }

    //
    // Switch Off RED & Green LEDs to indicate that Device is
    // disconnected from AP and Simplelink is shutdown
    //
    GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    DBG_PRINT("GET_WEATHER: Test Complete\n\r");

    //
    // Loop here
    //
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t      CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
  //
  // Enable Processor
  //
  MAP_IntMasterEnable();
  MAP_IntEnable(FAULT_SYSTICK);

  PRCMCC3200MCUInit();
}

//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the SLHost task
//!    2. Invokes the GetWeather Task
//!
//! \return None.
//
//****************************************************************************
void main()
{
    long lRetVal = -1;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Pinmux for UART
    //
    PinMuxConfig();

    //
    // Initializing DMA
    //
    UDMAInit();
#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
#endif
    //
    // Display Application Banner
    //
    //DisplayBanner(APP_NAME);
    //DisplayBanner(OTA_VENDOR_STRING);
    DisplayBanner("Frmwr 3");

    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the GetWeather task
    //
    lRetVal = osi_TaskCreate(GetWeatherTask,
                    (const signed char *)"Get Weather",
                    OSI_STACK_SIZE, 
                    NULL, 
                    1, 
                    NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the task scheduler
    //
    osi_start();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
