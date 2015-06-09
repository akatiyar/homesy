/*
 * main_M.c
 *
 *  Created on: 01-Jun-2015
 *      Author: Chrysolin
 */



#include "include_all.h"
#include "app.h"
#include "math.h"
#include "timer_fns.h"

uint8_t cnt_prakz2;

struct ProjectGlobals globals;
struct MQXLiteGlobals mqxglobals;

extern struct SV_6DOF_GB_BASIC thisSV_6DOF_GB_BASIC;

extern volatile uint32_t v_OneSecFlag;

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

	int i = 0;
	while(1)
	{
		// Need to wait for here till the next (1/200Hz) gets over

		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();
		//UART_PRINT("r\n\r");

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
			//UART_PRINT("f\n\r");
			i++;
		}

		//cnt_prakz2++;

		if(mqxglobals.MagCal_Event_Flag == 1)
		//if(cnt_prakz2>60)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
			//UART_PRINT("m* %d\n\r", i);
			cnt_prakz2 = 0;
		}

		UtilsDelay(.25*8000000/6);
	}
}
volatile uint8_t flag = 0;

void fxos_main_waitfor40degrees()
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

	int i = 0;
	while(1)
	{
		// Need to wait for here till the next (1/200Hz) gets over

		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();
		//UART_PRINT("r\n\r");

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
			//UART_PRINT("f\n\r");
			i++;
			if(thisSV_6DOF_GB_BASIC.fLPRho < 20)
			{
				flag = 1;
			}
			if(flag == 1)
			{
				if(thisSV_6DOF_GB_BASIC.fLPRho > 350)
				{
					flag = 0;
					UART_PRINT("Door cndtn met: %3.2f",thisSV_6DOF_GB_BASIC.fLPRho);
					return;
				}
			}
		}

		if(mqxglobals.MagCal_Event_Flag == 1)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
			//UART_PRINT("m* %d\n\r", i);
		}

		//UtilsDelay(.25*80000000/6);
	}
}

int32_t fxos_calibrate()
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

	int i = 0;

	//InitializeTimer();
	//StartTimer();
	//while(!v_OneSecFlag)
	while(1)
	{
		// Need to wait for here till the next (1/200Hz) gets over

		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();
		//UART_PRINT("r\n\r");

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
			//UART_PRINT("f\n\r");
		}

		if(mqxglobals.MagCal_Event_Flag == 1)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
			//UART_PRINT("m* %d\n\r", i);
		}

		//while(mqxglobals.Sampling_Event_Flag == 0);
		UtilsDelay(.005*80000000/6);
		i++;
		if(i == 2000)
		{
			break;
		}
	}
	//StopTimer();

	return 1;
}

int32_t fxos_get_initialYaw(float_t* pfInitYaw)
{
	uint8_t gotAngleFlag = 0;

//Comment off if calling fxos_calibrate() before
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

	int32_t i = 0;
	while(!gotAngleFlag)
	{
		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
			if(i++ > 200)
			{
				gotAngleFlag = 1;
			}
		}
		UtilsDelay(.005*80000000/6);
	}

	*pfInitYaw = thisSV_6DOF_GB_BASIC.fLPRho;

	return 1;
}


int32_t fxos_waitFor40Degrees(float_t fYaw_closedDoor)
{
	float_t fYaw_U, fYaw_L, fYaw;
	uint8_t Case_L, Case_U;
	uint8_t flag_U = 0, flag_L = 0;

#define UPPER_DEGREE 60.0F
#define LOWER_DEGREE 45.0F

#define CASE_NO_360CCROSSOVER	0
#define CASE_360CCROSSOVER		1

	fYaw_U = fYaw_closedDoor - UPPER_DEGREE;
	if( 0 > fYaw_U )
	{
		fYaw_U += 360;
		Case_U = CASE_360CCROSSOVER;
	}
	else
	{
		Case_U = CASE_NO_360CCROSSOVER;
	}

	fYaw_L = fYaw_closedDoor - LOWER_DEGREE;
	if( 0 > fYaw_L )
	{
		fYaw_L += 360;
		Case_L = CASE_360CCROSSOVER;
	}
	else
	{
		Case_L = CASE_NO_360CCROSSOVER;
	}

	//Hard coding angle values
	fYaw_U = 300;
	fYaw_L = 309;
	Case_U = CASE_NO_360CCROSSOVER;
	Case_L = CASE_NO_360CCROSSOVER;

	UART_PRINT("Closed Door: %3.2f\n\r", fYaw_closedDoor);
	UART_PRINT("U: %3.2f, Case: %x\n\r", fYaw_U, Case_U);
	UART_PRINT("L: %3.2f, Case: %x\n\r", fYaw_L, Case_L);

	while(!flag_L)
	{
		// Need to wait for here till the next (1/200Hz) gets over

		mqxglobals.RunKF_Event_Flag = 0;
		// read the sensors
		RdSensData_Run();
		//UART_PRINT("r\n\r");

		mqxglobals.MagCal_Event_Flag = 0;
		if(mqxglobals.RunKF_Event_Flag == 1)
		{
			// call the sensor fusion algorithms
			Fusion_Run();
			//UART_PRINT("f\n\r");

			fYaw = thisSV_6DOF_GB_BASIC.fLPRho;

			if(!flag_U)
			{
				switch(Case_U)
				{
				case CASE_NO_360CCROSSOVER:
					if((fYaw <= fYaw_U)||(fYaw > (fYaw_closedDoor-5)))
						flag_U = 1;
					break;
				case CASE_360CCROSSOVER:
					if((fYaw <= fYaw_U)&&(fYaw > (fYaw_closedDoor-5)))
						flag_U = 1;
					break;
				}
			}
			else if(!flag_L)
			{
				MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off //Temp
				switch(Case_L)
				{
				case CASE_NO_360CCROSSOVER:
					if((fYaw >= fYaw_L)&&(fYaw < (fYaw_closedDoor+5)))
						flag_L = 1;
					break;
				case CASE_360CCROSSOVER:
					if((fYaw >= fYaw_L)||(fYaw < (fYaw_closedDoor+5)))
						flag_L = 1;
					break;
				}
			}

		}

		if(mqxglobals.MagCal_Event_Flag == 1)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
			//UART_PRINT("m %d\n\r", i);
		}

		//while(mqxglobals.Sampling_Event_Flag == 0);
		//UtilsDelay(.25*80000000/6);
		//i++;
		UtilsDelay(.005*80000000/6);
	}

	return 1;
}
