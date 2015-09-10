/*
 * ota_update.c
 *
 *  Created on: 03-Jul-2015
 *      Author: Chrysolin
 */


#include "ota.h"
#include "app_common.h"
#include "simplelink.h"
#include "network_related_fns.h"
//Tag:OTA
#include "flc_api.h"
#include "ota_api.h"

//Tag:OTA
#define OTA_SERVER_NAME                 "api.dropbox.com"
#define OTA_SERVER_IP_ADDRESS           0x00000000
#define OTA_SERVER_SECURED              1
#define OTA_SERVER_REST_UPDATE_CHK      "/1/metadata/auto/" // returns files/folder list
#define OTA_SERVER_REST_RSRC_METADATA   "/1/media/auto"     // returns A url that serves the media directly
#define OTA_SERVER_REST_HDR             "Authorization: Bearer "
//#define OTA_SERVER_REST_HDR_VAL         "gpmUoDDvCoAAAAAAAAAABd0vZXQLq5Br36HLFYl45ssi4ovc58y9G4xEET39CjKc"	//ULPC_OTA
//#define OTA_SERVER_REST_HDR_VAL         "gpmUoDDvCoAAAAAAAAAABz3goxouL7gTfYq9KZZMcNYMES8mwG_cihHC-lbiWd8f"		//TISDK_OTA, Get Waether
#define OTA_SERVER_REST_HDR_VAL         "gpmUoDDvCoAAAAAAAAAADswjggdTIQeGiBd559fnFNkNd01HLQp_geGdXvihda5f"//Vcognition OTA
#define LOG_SERVER_NAME                 "api-content.dropbox.com"
#define OTA_SERVER_REST_FILES_PUT       "/1/files_put/auto/"
//#define OTA_VENDOR_STRING               "Vid00_Pid00_Ver00"
//#define OTA_VENDOR_STRING               "TI_CC3200_WTHR1"


//Tag:OTA
static OtaOptServerInfo_t g_otaOptServerInfo;
static void *pvOtaApp;

extern int32_t NWP_SwitchOn();
extern int32_t NWP_SwitchOff();

int OTAServerInfoSet(void **pvOtaApp, char *vendorStr);
//static void RebootMCU();

static int32_t OTA_Init();
static int32_t OTA_Run();
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
//	This function initializes OTA. Call this before OTA_Run
//
//	NOTE: MWP should be on while calling this function
//****************************************************************************
static int32_t OTA_Init()
{
    unsigned char ucVendorStr[20];

    //Tag:OTA
    //
	// Initialize OTA
	//
	pvOtaApp = sl_extLib_OtaInit(RUN_MODE_NONE_OS | RUN_MODE_BLOCKING,0);	//Other parameters are not supported
	//
	// Initializa OTA service
	//
	strcpy((char *)ucVendorStr,OTA_VENDOR_STRING);
    OTAServerInfoSet(&pvOtaApp,(char *)ucVendorStr);

    return 0;
}

//****************************************************************************
//	This function does OTA update, that is, looks for updates, downloads
//updates (if available) and resets the device
//
//Note: 1. Call OTA_Init() before calling this funcitons.
//		2. NWP has to be on while calling this fn
//****************************************************************************
static int32_t OTA_Run()
{
	int32_t lRetVal;
	int SetCommitInt;
	long OptionLen;
    unsigned char OptionVal;

	lRetVal = 0;

    //Tag:OTA
    //
    //	Check if this image is booted in test mode. If yes, commit the image in flash
    //
    sl_extLib_OtaGet(pvOtaApp, EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT, &OptionLen,(_u8*)&OptionVal);
    DEBG_PRINT("IS_PENDING_COMMIT? %d\n\r", OptionVal);
    if(OptionVal == true)
    {
    	RELEASE_PRINT("OTA: Older firmware commit Pending. Press button for next OTA update on next reset\n\r");
    	SetCommitInt = OTA_ACTION_IMAGE_COMMITED;
    	sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_COMMIT, sizeof(int),(_u8 *)&SetCommitInt);
        //RebootMCU();
    	//PRCMSOCReset();
    }
    else
    {
    	//LED_Blink_2(.2,1,BLINK_FOREVER);

    	while(!lRetVal)	//lRetVal is non-zero only if OTA download is complete or an error has occured
    	{
    		lRetVal = sl_extLib_OtaRun(pvOtaApp);
    	}

    	DEBG_PRINT("state = %d\n", lRetVal);
    	if(lRetVal < 0)
    	{
    		RELEASE_PRINT("Error with OTA Server\n");
    	}
    	else if(lRetVal == RUN_STAT_NO_UPDATES)
    	{
    		RELEASE_PRINT("No updates\n");
    	}
    	else if((lRetVal & RUN_STAT_DOWNLOAD_DONE))
    	{
    		//
    		//	Set OTA File for testing
    		//
    		lRetVal = sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_TEST, sizeof(int), (_u8 *)&SetCommitInt);
    		RELEASE_PRINT("New firmware downloaded\n");
    		RELEASE_PRINT("Rebooting...\n");
    		UtilsDelay(12*80000/6);	//12 milli sec
    		PRCMSOCReset();
    		//RebootMCU();
    	}
    }
	return 0;
}

//******************************************************************************
//This function downloads any available updates from Dropbox and resets the
//device, so it begins running the downloaded code.
//******************************************************************************
int32_t OTA_Update()
{
	RELEASE_PRINT("OTA: Update\n");
	//
	//	Connect cc3200 to wifi AP
	//
	//ConfigureSimpleLinkToDefaultState();
	//sl_Start(0, 0, 0);
	//ConnectToNetwork_STA();
	WiFi_Connect();

	OTA_Init();
	OTA_Run();

	NWP_SwitchOff();

	return 0;
}
//******************************************************************************
//This function commits the new firmware (itself) if it is running in test
//mode (first time)
//******************************************************************************
int32_t OTA_CommitImage()
{
	int SetCommitInt;
	long OptionLen;
    unsigned char OptionVal;
    long lRetVal;

	//sl_Start(0,0,0);
    NWP_SwitchOn();

	OTA_Init();
    //Tag:OTA
    //
    //	Check if this image is booted in test mode
    //
    sl_extLib_OtaGet(pvOtaApp, EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT, &OptionLen,(_u8*)&OptionVal);
    DEBG_PRINT("IS_PENDING_COMMIT? %d\n", OptionVal);
    if(OptionVal == true)
    {
    	RELEASE_PRINT("Committing new firmware\n");
    	SetCommitInt = OTA_ACTION_IMAGE_COMMITED;
    	lRetVal = sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_COMMIT, sizeof(int),(_u8 *)&SetCommitInt);
        if (lRetVal == SUCCESS)
        {
        	lRetVal = NEW_FIRMWARE_COMMITTED;
        }
    	//RebootMCU();
    	//PRCMSOCReset();
    	//Can upload an object to parse informing of the ota firmware update
    }
    else
    {
    	lRetVal = NO_COMMIT;
    }

    NWP_SwitchOff();

    return lRetVal;
}
