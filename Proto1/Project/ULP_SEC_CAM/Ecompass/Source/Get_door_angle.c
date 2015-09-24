/*
 * Get_door_angle.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include <ecompass.h>

float_t get_angle()
{
	while(1)
	{
		mqxglobals.RunKF_Event_Flag = 0;

//		start_100mSecTimer();

		// read the sensors

		//The 2.5ms interval started sometime back, if doPeriodicAction is hi.
		//Can remove this if part if the time taken for the algo etc are
		//kept below the sampling rate of magnetometer
		if(doPeriodicAction_flag)
		{
			//DEBG_PRINT("Hi\n");
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

		//DEBG_PRINT("r\n\r");
//		ulTimeDuration_ms = get_timeDuration();
//		stop_100mSecTimer();
//		DEBG_PRINT("Read SensData - %d ms\n\r", ulTimeDuration_ms);
//		start_100mSecTimer();

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
//			DEBG_PRINT("f\n\r");
//			DEBG_PRINT("CompassVal(rho):%f, phi:%f, psi:%f, theta:%f\n\r",
//							thisSV_6DOF_GB_BASIC.fLPRho, thisSV_6DOF_GB_BASIC.fLPPhi,
//							thisSV_6DOF_GB_BASIC.fLPPsi, thisSV_6DOF_GB_BASIC.fLPThe);
			return thisSV_6DOF_GB_BASIC.fLPRho;
		}
	}
}
