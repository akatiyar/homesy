/*
 * get_compass_angle_from_magnetometer.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <accelomtr_magntomtr_fxos8700.h>
#include "app_common.h"
#include "include_all.h"
#include "flash_files.h"
#include "timer_fns.h"
#include "ecompass.h"

extern float_t* g_pfUserConfigData;

extern int16_t angleCheck_Initializations();
extern int16_t magnetometer_initialize();
extern float_t get_angle();

//******************************************************************************
//	Collects ecompass value and stores the same in flash
//
//	param	ucAngle - can be either ANGLE_40 or ANGLE_90. This parameter
//				determines the offset within the flash file where the compass
//				value will be stored
//******************************************************************************
int32_t CollectAngle(uint8_t ucAngle)
{
	int32_t lRetVal = -1;
	float_t fAngleTemp;

	//Initialize e-compass
	angleCheck_Initializations();

	//Timer for reading samples from magnetometer (output data rate)
	start_periodicInterrupt_timer(2.5);

	//Initialize FXOS device, e-compass buffer
	magnetometer_initialize();

	// Get angle from E-compass <- magnetometer chip
	fAngleTemp = get_angle();

	//Stop the magnetometer sample collect rate timer
	stop_periodicInterrupt_timer();

	// Compensation, to avoid angle complement on slight tilt from the verical
	if(thisSV_6DOF_GB_BASIC.fLPPhi<0)
	{
		fAngleTemp = thisSV_6DOF_GB_BASIC.fLPRho +180;
		if(fAngleTemp>360)
		{
			fAngleTemp = fAngleTemp-360;
		}
	}

	//Save configs in the global variable
	if(ucAngle == ANGLE_90)
	{
		//DEBG_PRINT("90deg: %f\n\r",fAngleTemp);
		//DEBG_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_90/sizeof(float))) = fAngleTemp;
		RELEASE_PRINT("90deg: %3.2f\n",g_pfUserConfigData[(OFFSET_ANGLE_90/sizeof(float))]);
	}
	else if (ucAngle == ANGLE_40)
	{
		//DEBG_PRINT("40deg: %f\n\r",fAngleTemp);
		//DEBG_PRINT("90deg: %f, 40deg: %f\n\r",g_pfUserConfigData[0], g_pfUserConfigData[1]);
		*(g_pfUserConfigData+(OFFSET_ANGLE_40/sizeof(float))) = fAngleTemp;
		//DEBG_PRINT("40deg: %f\n\r",g_pfUserConfigData[1]);
		RELEASE_PRINT("40deg: %3.2f\n",g_pfUserConfigData[(OFFSET_ANGLE_40/sizeof(float))]);
	}

	return lRetVal;
}
