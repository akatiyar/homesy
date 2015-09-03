/*
 * SendObject_ToParse.c
 *
 *  Created on: 19-Aug-2015
 *      Author: Chrysolin
 */

#include "app_common.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "appFns.h"

extern float g_angleOffset_to180;
extern float gdoor_90deg_angle;
extern float gdoor_40deg_angle;

extern int32_t NWP_SwitchOff();
void Calculate_TrueMinMaxAngles();

int32_t SendObject_ToParse(uint8_t ucClassName)
{
	ParseClient clientHandle;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];
	int32_t lRetVal;
	struct u64_time time_now;

	//LED_On();

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		NWP_SwitchOff();
		ASSERT_ON_ERROR(lRetVal);
	}

	LED_Blink_2(.2,1,BLINK_FOREVER);

	//	Parse initialization
	clientHandle = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device

	switch(ucClassName)
	{
		case GROUND_DATA:
		{
			// Collect remaining applicable timestamps
			if(IsLightOff(LUX_THRESHOLD))
			{
				cc_rtc_get(&time_now);
				g_TimeStamp_DoorClosed = time_now.secs * 1000 + time_now.nsec / 1000000;
			}
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


