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

extern uint8_t print_count;
extern uint8_t valid_case;

struct ProjectGlobals globals;
struct MQXLiteGlobals mqxglobals;

//******************************************************************************
//	Initialization of variables pertaining to angle check
//
//	This function sets
//		1. the initializes global variables of sensor fusion libraryr and
//			snap-position detection algorithm
//		2. reads magnetometer calibration values and config door angles from
//			flash
//
//	NOTE: This function expects NWP to be on before the funciton is called
//
// Call angleCheck_Initializations() and magnetometer_initialize() before
//		getting e-compass angles
//******************************************************************************
int16_t angleCheck_Initializations()
{
	float_t Mag_Calb_Value[MAGNETOMETER_DATA_SIZE/sizeof(float)];
	uint8_t tmpCnt=0;

	//
	// Initialize global variables of snap-position detection algorithm
	//
	g_flag_door_closing_45degree = 0;
	valid_case = 0;
	print_count = 0;

	//
	// Initialize ecompass library globals
	//
	globals.iPacketNumber = 0;
	globals.AngularVelocityPacketOn = true;
	globals.DebugPacketOn = true;
	globals.RPCPacketOn = true;
	globals.AltPacketOn = true;
	globals.iMPL3115Found = false;
	globals.MagneticPacketID = 0;

	//
	// Initialize the sensor fusion algorithms
	//
	Fusion_Init();	// E-compass library fn

	//
	//	Get the magnetometer calibrations and Config angles from flash
	//
	ReadFile_FromFlash((uint8_t*)Mag_Calb_Value, (uint8_t*)USER_CONFIGS_FILENAME, MAGNETOMETER_DATA_SIZE, 0);
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

	// **Used for door min & max angles detection only
	//Calculate the offset that will bring door 40 degree angle to 180degrees.
	//Offsetting ecompass angles by this value will ensure all valid door angles
	//are in Q2 and Q3. Therefore, there will be no crossovers.
	g_angleOffset_to180 = 180.0 - gdoor_40deg_angle;

	// Print config angles
	DEBG_PRINT("Magn Data from Flash File:\n");
	DEBG_PRINT("90: %3.2f\n", gdoor_90deg_angle);
	DEBG_PRINT("40: %3.2f\n", gdoor_40deg_angle);
	DEBG_PRINT("Offset to 180: %3.2f\n", g_angleOffset_to180);

	return 0;
}

//******************************************************************************
//	Magnetometer FXOS8700 register configuration and intial buffer filling
//
//	This function
//		1. writes to the Magnetometer to initialize the device
//		2. first few angle calculations are read out, just to be safe to avoid
//			first few invalid
//
//	NOTE: 1. This function calls get_anlge() so, periodic timer should be on,
//			before calling this function
//		  2. Call this after angleCheck_Initializations, because only then
//			the calibration values will be applied
//
// Call angleCheck_Initializations() and magnetometer_initialize() before
//		getting e-compass angles
//******************************************************************************
int16_t magnetometer_initialize()
{
	//
	// initialize the physical sensors over I2C and the sensor data structures
	//
	RdSensData_Init();	// E-compass library fn

	//
	//5 get_angle calls are made to eliminate initial few angle values that could
	//be wrong. Just to be safe. May not be necessary at all.
	//
	int32_t i;
	for(i=0; i<5; i++)
	{
		get_angle();
	}

	return 0;
}

//******************************************************************************
//	Initializations needed for mangetometer calibration
//
//	This function
// 		(i) initilaizes Sensor-fusion library globals
//		(ii) configure Magnetometer registers
//******************************************************************************
int16_t fxosDefault_Initializations()
{
	g_flag_door_closing_45degree = 0;	//not needed

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
