/*
 * fxos8700_calibrate.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

//******************************************************************************
//	Magnetometer calibration
//	This function:
//		1. Reads sensor data
//		2. update magnetometer buffer
//		3. Runs calibration function
//	Call this function repeatedly for calibrating the magnetometer
//******************************************************************************
int16_t fxos_Calibration()
{
	mqxglobals.RunKF_Event_Flag = 0;

	// read the sensors
	RdSensData_Run();

	mqxglobals.MagCal_Event_Flag = 0;
	if(mqxglobals.RunKF_Event_Flag == 1)
	{
		// call the sensor fusion algorithm to update magnetometer buffer
		Fusion_Run();
	}

	if(mqxglobals.MagCal_Event_Flag == 1)
	{
		// call the calibration algorithm
		MagCal_Run(&thisMagCal, &thisMagBuffer);
	}

	return 0;
}
