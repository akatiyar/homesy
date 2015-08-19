/*
 * SendObject_ToParse.c
 *
 *  Created on: 19-Aug-2015
 *      Author: Chrysolin
 */

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

	LED_On();

	//Connect to WiFi
	lRetVal = WiFi_Connect();
	if (lRetVal < 0)
	{
		NWP_SwitchOff();
		ASSERT_ON_ERROR(lRetVal);
	}

	//	Parse initialization
	clientHandle = InitialiseParse();

	Get_FridgeCamID(&ucFridgeCamID[0]);	//Get FridgeCam ID from unique MAC
										//ID of the CC3200 device

	switch(ucClassName)
	{
		case GROUND_DATA:
		{
			Calculate_TrueMinMaxAngles();

			// Collect remaining applicable timestamps
			if(IsLightOff(LUX_THRESHOLD))
			{
				cc_rtc_get(&time_now);
				g_TimeStamp_DoorClosed = time_now.secs * 1000 + time_now.nsec / 1000000;
			}
			DEBG_PRINT("%ld, %ld, %ld\n",g_TimeStamp_MaxAngle,g_TimeStamp_MinAngle, g_TimeStamp_SnapAngle);
			g_TimeStamp_MaxAngle = g_Struct_TimeStamp_MaxAngle.secs * 1000 + g_Struct_TimeStamp_MaxAngle.nsec / 1000000;
			g_TimeStamp_MinAngle = g_Struct_TimeStamp_MinAngle.secs * 1000 + g_Struct_TimeStamp_MinAngle.nsec / 1000000;
			g_TimeStamp_SnapAngle = g_Struct_TimeStamp_SnapAngle.secs * 1000 + g_Struct_TimeStamp_SnapAngle.nsec / 1000000;
			DEBG_PRINT("%ld, %ld, %ld\n",g_TimeStamp_MaxAngle,g_TimeStamp_MinAngle, g_TimeStamp_SnapAngle);

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

//Calculate min and max door angles by (1)reversing offset and //handling out of range angle(negative and >360) (2)swappping //min and max incase Angle90<Angle40
void Calculate_TrueMinMaxAngles()
{
	float temp_swap_variable;

	DEBG_PRINT("Calculate_TrueMinMaxAngles()\n");
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Reverse offset
	g_fMinAngle -= g_angleOffset_to180;
	g_fMaxAngle -= g_angleOffset_to180;
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Handle <0 and >360 cases
	if(g_fMaxAngle < 0)
	{
		g_fMaxAngle += 360;
	}
	else if(g_fMaxAngle > 360)
	{
		g_fMaxAngle -= 360;
	}

	if(g_fMinAngle < 0)
	{
		g_fMinAngle += 360;
	}
	else if(g_fMinAngle > 360)
	{
		g_fMinAngle -= 360;
	}
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Swap the min and max angles if Angle90<Angle40
	if (((gdoor_90deg_angle < gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) < 120))
		|| ((gdoor_90deg_angle > gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) > 120)))
	{
		temp_swap_variable = g_fMinAngle;
		g_fMinAngle = g_fMaxAngle;
		g_fMaxAngle = temp_swap_variable;
	}
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	return;
}
