/*
 * fxos8700_calibrate.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

int16_t fxos_Calibration()
{
	mqxglobals.RunKF_Event_Flag = 0;

	// read the sensors
	RdSensData_Run();

	mqxglobals.MagCal_Event_Flag = 0;
	if(mqxglobals.RunKF_Event_Flag == 1)
	{
		// call the sensor fusion algorithm
		Fusion_Run();
	}

	if(mqxglobals.MagCal_Event_Flag == 1)
	{
		// call the calibration algorithm
		MagCal_Run(&thisMagCal, &thisMagBuffer);
		//DEBG_PRINT("m* %d\n\r", i);
	}

	return 0;
}
