/*
 * Min_max_check.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

//******************************************************************************
// Checks if the angle passed as parameter is min/max. If it is min/max it is
//	stored in min/max angle global variable and the current timestamp is also
//	stored in the corresponding timestamp variable
//******************************************************************************
void Check_MinMax(float_t angle_avg)
{
	float angle_reg_afterOffset = 0;

	//Offset to get angles in Q2 and Q3
	angle_reg_afterOffset = angle_avg + g_angleOffset_to180;
	if(angle_reg_afterOffset > 360)
	{
		angle_reg_afterOffset -= 360;
	}
	else if(angle_reg_afterOffset < 0)
	{
		angle_reg_afterOffset += 360;
	}

	// Invalid case. Angle ramping up from 0 to 360 or down from 360 to 0
//	if((angle_reg_afterOffset > 240)||(angle_reg_afterOffset < 120))
//	{
//		return;
//	}

	if(angle_reg_afterOffset < g_fMinAngle)
	{
		g_fMinAngle = angle_reg_afterOffset;
		//g_RawMinAngle = angle_avg;
//		DEBG_PRINT("N");
		//DEBG_PRINT("MIN: %d\n", g_fMinAngle);
		cc_rtc_get(&g_Struct_TimeStamp_MinAngle);
	}
	if (angle_reg_afterOffset > g_fMaxAngle)
	{
		g_fMaxAngle = angle_reg_afterOffset;
		//g_RawMaxAngle = angle_avg;
//		DEBG_PRINT("X");
		//DEBG_PRINT("MAX: %d\n", g_fMaxAngle);
		cc_rtc_get(&g_Struct_TimeStamp_MaxAngle);
	}

	if(angle_avg < g_RawMinAngle)
	{
		g_RawMinAngle = angle_avg;
	}
	if(angle_avg > g_RawMaxAngle)
	{
		g_RawMaxAngle = angle_avg;
	}

	return;
}
