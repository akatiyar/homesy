/*
 * main_M.c
 *
 *  Created on: 01-Jun-2015
 *      Author: Chrysolin
 */



#include "include_all.h"

struct ProjectGlobals globals;
struct MQXLiteGlobals mqxglobals;

void fxos_main()
{
	// initialize globals
	globals.iPacketNumber = 0;
	globals.AngularVelocityPacketOn = true;
	globals.DebugPacketOn = true;
	globals.RPCPacketOn = true;
	globals.AltPacketOn = true;
	globals.iMPL3115Found = false;
	globals.MagneticPacketID = 0;

	// initialize the physical sensors over I2C and the sensor data structures
	RdSensData_Init();

	// initialize the sensor fusion algorithms
	Fusion_Init();


	while(1)
	{
		// Need to wait for here till the next (1/200Hz) gets over

		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
		}

		if(mqxglobals.MagCal_Event_Flag == 1)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
		}

		//UtilsDelay(.25*80000000/6);
	}
}
