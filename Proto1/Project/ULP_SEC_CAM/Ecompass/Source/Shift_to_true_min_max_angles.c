/*
 * Shift_to_true_min_max_angles.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>
#include "app.h"

//******************************************************************************
//	Calculates the true min/max door angles from the offset_to180 min/max angles
//
//STEPS:
//	(1)reversing offset and handling out-of-range angle(negative and >360)
//	(2)swappping min and max incase Angle90<Angle40
//Swaps timestamps also if door angles are swapped
//******************************************************************************
void Calculate_TrueMinMaxAngles()
{
	float temp_swap_variable;
	long long temp_swap_variable_uint64;

	DEBG_PRINT("TrueMinMaxAngles()\n");
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
	if (((gdoor_90deg_angle < gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) < A40_A90_DIFFMAG_MAX))
		|| ((gdoor_90deg_angle > gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) > A40_A90_DIFFMAG_MAX)))
	{
		temp_swap_variable = g_fMinAngle;
		g_fMinAngle = g_fMaxAngle;
		g_fMaxAngle = temp_swap_variable;

		temp_swap_variable_uint64 = g_TimeStamp_MaxAngle;
		g_TimeStamp_MaxAngle = g_TimeStamp_MinAngle;
		g_TimeStamp_MinAngle = temp_swap_variable_uint64;
	}
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	return;
}

