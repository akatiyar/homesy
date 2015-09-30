/*
 * SendObject_ToParse.c
 *
 *  Created on: 19-Aug-2015
 *      Author: Chrysolin
 */

#include <app_fns.h>
#include "app_common.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "nwp.h"

void Calculate_TrueMinMaxAngles();

//******************************************************************************
//	This function sends objects to parse
//	param[in] ucClassName - Class of the object to be sent. Can be USER_CONFIGS/
//							GROUND_DATA/FIRMWARE_VER
//******************************************************************************
int32_t SendObject_ToParse(uint8_t ucClassName)
{
	ParseClient clientHandle;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];
	int32_t lRetVal;
	struct u64_time time_now;

	//If ground data object is to be sent, the door closed timestamp is best
	//recorded asap
	if(ucClassName == GROUND_DATA)
	{
		// Collect remaining applicable timestamps
		if(IsLightOff(LUX_THRESHOLD))
		{
			cc_rtc_get(&time_now);
			g_TimeStamp_DoorClosed = time_now.secs * 1000 + time_now.nsec / 1000000;
		}
	}

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		NWP_SwitchOff();
		RETURN_ON_ERROR(lRetVal);
	}

	//	Parse initialization
	clientHandle = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device

	switch(ucClassName)
	{
		case GROUND_DATA:
		{
			//DEBG_PRINT("%ld, %ld, %ld, %ld\n",g_TimeStamp_MaxAngle, g_TimeStamp_MinAngle, g_TimeStamp_SnapAngle, g_TimeStamp_OpenAngle);
			g_TimeStamp_MaxAngle = g_Struct_TimeStamp_MaxAngle.secs * 1000 + g_Struct_TimeStamp_MaxAngle.nsec / 1000000;
			g_TimeStamp_MinAngle = g_Struct_TimeStamp_MinAngle.secs * 1000 + g_Struct_TimeStamp_MinAngle.nsec / 1000000;
			g_TimeStamp_SnapAngle = g_Struct_TimeStamp_SnapAngle.secs * 1000 + g_Struct_TimeStamp_SnapAngle.nsec / 1000000;
			g_TimeStamp_OpenAngle = g_Struct_TimeStamp_OpenAngle.secs * 1000 + g_Struct_TimeStamp_OpenAngle.nsec / 1000000;
			//DEBG_PRINT("%ld, %ld, %ld, %ld\n",g_TimeStamp_MaxAngle, g_TimeStamp_MinAngle, g_TimeStamp_SnapAngle, g_TimeStamp_OpenAngle);

			Calculate_TrueMinMaxAngles();

			g_fBatteryVolt_atEnd = Get_BatteryVoltage();

			UploadGroundDataObjectToParse(clientHandle, &ucFridgeCamID[0]);

			break;
		}

		case USER_CONFIGS:
		{
			UploadUserConfigObjectToParse(clientHandle, &ucFridgeCamID[0]);
			break;
		}

		case FIRMWARE_VER:
		{
			UploadFirmwareVersionObjectToParse(clientHandle, &ucFridgeCamID[0]);
			break;
		}
	}

	free((void*)clientHandle);	//malloc() in InitializeParse()

	NWP_SwitchOff();

	return 0;
}
