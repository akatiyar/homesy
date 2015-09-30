/*
 * door_angle_from_90_and_40.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>
#include "app.h"

//******************************************************************************
//Determines the OPEN angle by Interpolation from angle_40 and angle_90
//
//	param[in]	angle_40 - ecompass angle when the door is at 40 degrees from
//							the fridge
//	param[in]	angle_90 - ecompass angle when the door is at 90 degrees from
//							the fridge
//
//	return	ecompass angle when door is at 60 degrees (open angle) from fridge
//******************************************************************************
float_t Calculate_DoorOpenThresholdAngle(float_t angle_40, float_t angle_90)
{
	float_t angle_openThreshold, angle_40_temp = 0, angle_90_temp = 0;
	float_t offset;

	if(abs(angle_40-angle_90) < A40_A90_DIFFMAG_MAX)	//No 360 degree crossover
	{
		//Same interpolation formula works for both angle_90>angle_40 and
		//angle_90<angle_40
		angle_openThreshold = angle_40 + ((OPEN - FORTY_DEGREES) * ((angle_40 - angle_90)/(FORTY_DEGREES - NINETY_DEGREES)));
	}
	else		//i.e if there is a 360 degree crossover
	{
		if(angle_40 > angle_90) //i.e if angle_40 in Q4 (300s) and angle_90 in Q1 (<90)
		{
			//Calculate offset as the degrees between angle_40 and 360, plus a
			//margin to tip angle_40 over to Q1
			offset = (360 - angle_40 + 1);

			//Offset the angles so that crossover is eliminated
			angle_40_temp = angle_40 + offset; //should be 361 here. this can be removed
			angle_40_temp -= 360;	//should be 1 here. . this can be removed
			angle_90_temp += angle_90 + offset;

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
		}
		if(angle_90 > angle_40) //i.e if angle_90 in Q4 (300s) and angle_40 in Q1 (<90)
		{
			//Calculate offset as the degrees between angle_90 and 360, plus a
			//margin(1) to tip angle_90 over to Q1
			offset = (360 - angle_90 + 1);

			//Offset the angles so that crossover is eliminated
			angle_90_temp = angle_90 + offset; //should be 361 here. this can be removed
			angle_90_temp -= 360;	//should be 1 here. . this can be removed
			angle_40_temp = angle_40 + offset;

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
		}
	}

	DEBG_PRINT("60 degree: %f\n",angle_openThreshold);

	return angle_openThreshold;
}
