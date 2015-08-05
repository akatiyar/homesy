/*
 * sendUserConfigDataToParse.c
 *
 *  Created on: 31-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "flash_files.h"
#include "appFns.h"

extern float_t gdoor_OpenDeg_angle;
extern int32_t NWP_SwitchOff();

int32_t sendUserConfigData()
{
	ParseClient clientHandle1;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];
	int32_t lRetVal;

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		NWP_SwitchOff();
		ASSERT_ON_ERROR(lRetVal);
	}

	//	Parse initialization
	clientHandle1 = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device
	UploadUserConfigObjectToParse(clientHandle1, &ucFridgeCamID[0]);

	free((void*)clientHandle1);	//malloc() in InitializeParse()

	NWP_SwitchOff();

	return 0;

}


