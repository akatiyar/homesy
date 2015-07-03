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
#include "accelomtrMagntomtr_fxos8700.h"
#include "app_common.h"
#include "flash_files.h"
#include "camera_app.h"

uint8_t cnt_prakz2;
uint8_t g_flag_door_closing_45degree;
extern unsigned long g_image_buffer[(IMAGE_BUF_SIZE_BYTES/sizeof(unsigned long))];
extern float gdoor_90deg_angle;
extern float gdoor_40deg_angle;

extern void check_doorpos();
int16_t angleCheck();
int16_t angleCheck_Initializations();
float_t get_angle();
int16_t angleCheck_WithCalibration();

int16_t IsLightOff(uint16_t usThresholdLux);

struct ProjectGlobals globals;
struct MQXLiteGlobals mqxglobals;

extern struct SV_6DOF_GB_BASIC thisSV_6DOF_GB_BASIC;

extern volatile uint32_t v_OneSecFlag;

int16_t angleCheck_Initializations()
{
	float_t *Mag_Calb_Value = (float_t *) g_image_buffer;
	uint8_t tmpCnt=0;
	g_flag_door_closing_45degree = 0;

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

	ReadFile_FromFlash((uint8_t*)Mag_Calb_Value, (uint8_t*)FILENAME_ANGLE_VALS, MAX_FILESIZE_ANGLE_VALS, 0);

	gdoor_90deg_angle = Mag_Calb_Value[tmpCnt++];
	gdoor_40deg_angle  = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[0][0] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[0][1]=  Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[0][2] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[1][0] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[1][1] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[1][2] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[2][0] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[2][1] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.finvW[2][2] = Mag_Calb_Value[tmpCnt++];
	thisMagCal.fV[0]= Mag_Calb_Value[tmpCnt++];
	thisMagCal.fV[1]= Mag_Calb_Value[tmpCnt++];
	thisMagCal.fV[2]= Mag_Calb_Value[tmpCnt++];

	//UART_PRINT("90w:%3.2f\n\r",gdoor_90deg_angle);
	//UART_PRINT("40w:%3.2f\n\r",gdoor_40deg_angle);
	return 0;
}

int16_t fxosDefault_Initializations()
{
	g_flag_door_closing_45degree = 0;

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

	return 0;
}

int16_t angleCheck()
{
	mqxglobals.RunKF_Event_Flag = 0;
	// read the sensors
	RdSensData_Run();
	//UART_PRINT("r\n\r");

	mqxglobals.MagCal_Event_Flag = 0;
	if(mqxglobals.RunKF_Event_Flag == 1)
	{
		// call the sensor fusion algorithms
		Fusion_Run();
		check_doorpos();
		//UART_PRINT("f\n\r");
	}

	return 0;
}

int16_t fxos_Calibration()
{
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
	//if(cnt_prakz2>60)
	{
		MagCal_Run(&thisMagCal, &thisMagBuffer);
		//UART_PRINT("m* %d\n\r", i);
	}

	return 0;
}



float_t get_angle()
{
	while(1)
	{
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
			//UART_PRINT("CompassVal(rho):%f, phi:%f, psi:%f, theta:%f\n\r",
			//				thisSV_6DOF_GB_BASIC.fLPRho, thisSV_6DOF_GB_BASIC.fLPPhi,
			//				thisSV_6DOF_GB_BASIC.fLPPsi, thisSV_6DOF_GB_BASIC.fLPThe);
			return thisSV_6DOF_GB_BASIC.fLPRho;
		}
	}

}

void fxos_main()
{
	g_flag_door_closing_45degree = 0;

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
			check_doorpos();
			//UART_PRINT("f\n\r");
			i++;
			if(g_flag_door_closing_45degree)
			{
				standby_accelMagn_fxos8700();
				return;
			}
			if(IsLightOff(LUX_THRESHOLD))
				return;
		}

		//cnt_prakz2++;

		if(mqxglobals.MagCal_Event_Flag == 1)
		//if(cnt_prakz2>60)
		{
			MagCal_Run(&thisMagCal, &thisMagBuffer);
			//UART_PRINT("m* %d\n\r", i);
			cnt_prakz2 = 0;
		}
		//UtilsDelay(.25*8000000/6);
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
