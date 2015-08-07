/*
 * sendGroundData.c
 *
 *  Created on: 23-Jul-2015
 *      Author: Chrysolin
 */
#include "app_common.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "appFns.h"

extern int32_t NWP_SwitchOff();

int32_t SendGroundData()
{
	ParseClient clientHandle;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];
	int32_t lRetVal;
	struct u64_time time_now;

	LED_On();

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		NWP_SwitchOff();
		ASSERT_ON_ERROR(lRetVal);
	}

	// Collect remaining applicable timestamps
	if(IsLightOff(LUX_THRESHOLD))
	{
		cc_rtc_get(&time_now);
		g_TimeStamp_DoorClosed = time_now.secs * 1000 + time_now.nsec / 1000000;
	}
	g_TimeStamp_maxAngle = g_Struct_TimeStamp_MaxAngle.secs * 1000 + g_Struct_TimeStamp_MaxAngle.nsec / 1000000;
	g_TimeStamp_minAngle = g_Struct_TimeStamp_MinAngle.secs * 1000 + g_Struct_TimeStamp_MinAngle.nsec / 1000000;

	//	Parse initialization
	clientHandle = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device

	UploadGroundDataObjectToParse(clientHandle, &ucFridgeCamID[0]);

	free((void*)clientHandle);	//malloc() in InitializeParse()

	NWP_SwitchOff();

	return 0;
}
