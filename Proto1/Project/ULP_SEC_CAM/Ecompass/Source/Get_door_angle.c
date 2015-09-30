/*
 * Get_door_angle.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

//******************************************************************************
//	Get the ecompass angle
//
//	This function includes
//		1. Collecting 6-axis acceleration and magnetic fields from FXOS8700
//		2. Find compass heading from the 6-axis data
//
//	return	e-compass angle
//
//	The ecompass angle is also available in the global variable
//	thisSV_6DOF_GB_BASIC.fLPRho after this function call
//******************************************************************************
float_t get_angle()
{
	while(1)
	{
		mqxglobals.RunKF_Event_Flag = 0;

		//
		// Read 6-axis acceleration and magnetic field from FXOS8700
		//
		//The 2.5ms interval started sometime back, if doPeriodicAction is hi.
		//Can remove this if part if the time taken for the algo etc are
		//kept below the sampling rate of magnetometer
		if(doPeriodicAction_flag)
		{
			reload_periodicTimer();
		}
		else	//Usual case
		{
			while(!doPeriodicAction_flag)
			{
				//DEBG_PRINT("%d",doPeriodicAction_flag);
			}
		}
		RdSensData_Run();
		doPeriodicAction_flag = 0;

		//
		//	Find euler angles from 6-axis acceleration and magnetic field
		//
		mqxglobals.MagCal_Event_Flag = 0;	//@Should not be needed
		//mqxglobals.RunKF_Event_Flag is set when we have enough new samples(2) to
		//do fusion_run()
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
#ifdef PRINT_SENSOR_FUSION_ANGLES
			DEBG_PRINT("f\n\r");
			DEBG_PRINT("CompassVal(rho):%f, phi:%f, psi:%f, theta:%f\n\r",
							thisSV_6DOF_GB_BASIC.fLPRho, thisSV_6DOF_GB_BASIC.fLPPhi,
							thisSV_6DOF_GB_BASIC.fLPPsi, thisSV_6DOF_GB_BASIC.fLPThe);
#endif
			return thisSV_6DOF_GB_BASIC.fLPRho;
		}
	}
}
