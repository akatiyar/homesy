/*
 * main_M.c
 *
 *  Created on: 01-Jun-2015
 *      Author: Chrysolin
 */

#include <accelomtr_magntomtr_fxos8700.h>
#include <ecompass.h>
#include <include_all.h>
#include "app.h"
#include "math.h"
#include "timer_fns.h"
#include "app_common.h"
#include "flash_files.h"
#include "camera_app.h"

uint8_t cnt_prakz2;
extern uint8_t print_count;
extern uint8_t valid_case;

struct ProjectGlobals globals;
struct MQXLiteGlobals mqxglobals;

//******************************************************************************
//
//	This function sets the intial values of variable for ecompass angle finding.
//The function reads the magnetometer calibration values and door angles from
//Flash and expects NWP to be on before the funciton is called.
//
//******************************************************************************
int16_t angleCheck_Initializations()
{
	float_t Mag_Calb_Value[MAGNETOMETER_DATA_SIZE/sizeof(float)];
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

	// initialize the sensor fusion algorithms
	Fusion_Init();

	ReadFile_FromFlash((uint8_t*)Mag_Calb_Value, (uint8_t*)USER_CONFIGS_FILENAME, MAGNETOMETER_DATA_SIZE, 0);
	//DEBG_PRINT("Magn flash file read done\n");	//Remove when waketime optimization is over

	gdoor_90deg_angle = Mag_Calb_Value[(OFFSET_ANGLE_90/sizeof(float))];
	gdoor_40deg_angle  = Mag_Calb_Value[(OFFSET_ANGLE_40/sizeof(float))];

	tmpCnt = (OFFSET_MAG_CALB/sizeof(float));
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

	gdoor_OpenDeg_angle  = Mag_Calb_Value[(OFFSET_ANGLE_OPEN/sizeof(float))];

	//Calculate the offset that will bring door 40 degree angle to 180degrees. Offsetting ecompass angles by this value will ensure all valid door angles are in Q2 and Q3. Therefore there will be no crossovers.
	g_angleOffset_to180 = 180.0 - gdoor_40deg_angle;

	//g_fMinAngle = g_angleOffset_to180;
	//g_fMaxAngle = g_fMinAngle;

	DEBG_PRINT("Magn Data from Flash File:\n");
	DEBG_PRINT("90: %3.2f\n", gdoor_90deg_angle);
	DEBG_PRINT("40: %3.2f\n", gdoor_40deg_angle);
	DEBG_PRINT("Offset to 180: %3.2f\n", g_angleOffset_to180);

	print_count = 0;
	valid_case = 0;

//	DEBG_PRINT("90:%3.2f\n\r",gdoor_90deg_angle);
//	DEBG_PRINT("40:%3.2f\n\r",gdoor_40deg_angle);
//	DEBG_PRINT("Open:%3.2f\n\r",gdoor_OpenDeg_angle);

	return 0;
}

//******************************************************************************
//	This function writes to the Magnetometer to initialize the device.
//	Also, first few angle calculations are read out as a ork around.
//******************************************************************************
int16_t magnetometer_initialize()
{
	// initialize the physical sensors over I2C and the sensor data structures
	RdSensData_Init();

	//Tag:Work-around for first few invalid angle values
	//Tag:Remove timing stuff
	//uint32_t ulTimeDuration_ms;
	int32_t i;

//	start_100mSecTimer();

	//for(i=0; i<50; i++)	//50 is value based on observation of ecompass readings
	for(i=0; i<5; i++)	//5 reads are done is to eliminate the initially wrong
						//values
	{
		get_angle();
	}
//	ulTimeDuration_ms = get_timeDuration();
//	stop_100mSecTimer();
//	DEBG_PRINT("a+m init reading - %d ms\n\r", ulTimeDuration_ms);

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

