/*
 * sendGroundData.c
 *
 *  Created on: 23-Jul-2015
 *      Author: Chrysolin
 */
#include "app.h"
#include "network_related_fns.h"
#include "parse_uploads.h"

int32_t SendGroundData()
{
	ParseClient clientHandle1;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];
	int32_t lRetVal;

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		sl_Stop(0xFFFF);
		ASSERT_ON_ERROR(lRetVal);
	}

	//	Parse initialization
	clientHandle1 = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device

	UploadGroundDataObjectToParse(clientHandle1, &ucFridgeCamID[0]);

	free((void*)clientHandle1);	//malloc() in InitializeParse()

	sl_Stop(0xFFF);

	return 0;
}
