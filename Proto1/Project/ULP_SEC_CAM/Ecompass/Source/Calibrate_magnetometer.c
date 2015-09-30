/*
 * magnetometer_calibration.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <accelomtr_magntomtr_fxos8700.h>
#include "app_common.h"
#include "include_all.h"
#include "flash_files.h"

#define MAG_SENSOR_CALIBCOUNT		1	//Dont increase this. Increasing this
										//can cause funciton to be stuck in loop
										//indefinitely
#define MAX_CALIB_REPEATS			20
#define ACCEPTABLE_FITERROR			5

volatile uint8_t g_ucMagCalb;
struct MagCalibration thisMagCal;
extern float_t* g_pfUserConfigData;

int16_t fxos_Calibration();
int16_t fxosDefault_Initializations();

//******************************************************************************
//	This function does magnetometer calibration
//	Results are stored in global variable
//******************************************************************************
int32_t Get_Calibration_MagSensor()
{
	uint8_t tmpCnt=0;
	float_t previous_best_fit_error;
	uint8_t calibRepeatCnt;
	uint8_t i;

	previous_best_fit_error = 1000.0F;
	calibRepeatCnt = 0;

	// This for loop is to try MAX_CALIB_REPEATS trials of calibrations
	//Loop exits in one of the following cases
	//	1. if an acceptable fit error calibration is obtained before
	//MAX_CALIB_REPEATS trials (usual case)
	//	2. is button is pressed (user wants to abort case)
	//	3. Number of trials has reached max (no calibration gave fit error less
	//than acceptable value). Even in this case we save the best calibration
	//obtained in all the trials
	for(; calibRepeatCnt < MAX_CALIB_REPEATS; calibRepeatCnt++)
	{
		g_ucMagCalb = 0;

		//Initialize magnetometer and variables for sensor fusion library
		fxosDefault_Initializations();

		// One calibration trial
		while(g_ucMagCalb < MAG_SENSOR_CALIBCOUNT)
		//while( g_ucMagCalb < (calibRepeatCnt+1) )
		{
			//Call to sensor fusion library functions
			fxos_Calibration();

			//If by any chance the 20 iteraions are not happening inspite of user
			//rotating (should not happen in normal cases) in all directions, he
			//can press the exit button. The best calibration so far will be
			//saved. If the device was not rotated at all, the older
			//configuration already in the flash file will be retained
			//Better be replaced with a stop calibration button
			if(g_ucExitButton == BUTTON_PRESSED)
			{
				//g_ucExitButton = BUTTON_NOT_PRESSED;
				break;
			}
		}
		if(g_ucExitButton == BUTTON_PRESSED)
		{
			g_ucExitButton = BUTTON_NOT_PRESSED;
			break;
		}
		DEBG_PRINT("%d  %3.2f\n", calibRepeatCnt, thisMagCal.fFitErrorpc);

		// If fit error is less than the previous best fit error, save the
		//calibration details
		if (thisMagCal.fFitErrorpc < previous_best_fit_error)
		{
			//Save the calibration
			tmpCnt = (OFFSET_MAG_CALB/sizeof(float_t));
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][0];
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][1];
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[0][2] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][0] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][1] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[1][2] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][0] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][1] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.finvW[2][2] ;
			g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[0];
			g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[1];
			g_pfUserConfigData[tmpCnt++] = thisMagCal.fV[2];
			g_pfUserConfigData[(OFFSET_FIT_ERROR/sizeof(float_t))] = thisMagCal.fFitErrorpc;
			DEBG_PRINT("Taken\n");

			//Update previous_best_fit_error
			previous_best_fit_error = thisMagCal.fFitErrorpc;

			//If the fit error is within acceptable fit error range, exit loop
			if (thisMagCal.fFitErrorpc < ACCEPTABLE_FITERROR)
			{
				break;
			}
		}
	}

	for(i=OFFSET_MAG_CALB; i<12;i++)
		DEBG_PRINT("%3.2f\n",g_pfUserConfigData[i]);
	RELEASE_PRINT("%3.2f%%\n",g_pfUserConfigData[i]);

	//Switch off LED for 5 sec to indicate that the fit error was not within
	//acceptable limits
	if(thisMagCal.fFitErrorpc > ACCEPTABLE_FITERROR)
	{
		LED_Off();
		osi_Sleep(5000);	//5 seconds delay
	}

	return 0;
}
