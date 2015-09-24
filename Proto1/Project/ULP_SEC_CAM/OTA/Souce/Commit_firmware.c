/*
 * commit_new_image.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "simplelink.h"

#include "app_common.h"
#include "network_related_fns.h"

#include "flc_api.h"
#include "ota_api.h"
#include "ota.h"

extern void *pvOtaApp;
extern int32_t OTA_Init();

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

    NWP_SwitchOn();

	OTA_Init();
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
    }
    else
    {
    	lRetVal = NO_COMMIT;
    }

    NWP_SwitchOff();

    return lRetVal;
}
