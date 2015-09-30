/*
 * Min_max_check.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

//******************************************************************************
//	Checks if angle is min/max.
//
//	param[in] angle_avg - angle which has to be checked
//	Previous min/max are in global variables
//
//	In order to handle the case where the door crosses the 360 degree position,
//	we offset all the angle values so that all angle values will be within the
//	second and third quadrants.
//
//	OUTPUT: If it is min/max, it is stored in min/max angle global variable and
//	the current timestamp is also stored in the corresponding timestamp variable
//******************************************************************************
void Check_MinMax(float_t angle_avg)
{
	float angle_reg_afterOffset = 0;

	//Do offset to get angle values in Q2 and Q3 only, so that there are
	//no crossovers
	angle_reg_afterOffset = angle_avg + g_angleOffset_to180;
	if(angle_reg_afterOffset > 360)
	{
		angle_reg_afterOffset -= 360;
	}
	else if(angle_reg_afterOffset < 0)
	{
		angle_reg_afterOffset += 360;
	}

	//	If current angle is < min angle, update min angle
	if(angle_reg_afterOffset < g_fMinAngle)
	{
		g_fMinAngle = angle_reg_afterOffset;
		//DEBG_PRINT("N");
		cc_rtc_get(&g_Struct_TimeStamp_MinAngle);
	}
	//	If current angle is > max angle, update max angle
	if (angle_reg_afterOffset > g_fMaxAngle)
	{
		g_fMaxAngle = angle_reg_afterOffset;
		//DEBG_PRINT("X");
		cc_rtc_get(&g_Struct_TimeStamp_MaxAngle);
	}

	//Find the raw min and raw max angles. @Not of much use now.
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
