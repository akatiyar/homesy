/*
 * appfile_magnmtr.c
 *
 *  Created on: 20-May-2015
 *      Author: Chrysolin
 */
#include "app.h"
#include "accelomtrMagntomtr_fxos8700.h"
#include "flash_files.h"
#include "simplelink.h"
#include "timer_fns.h"

int32_t Collect_InitMangReadings();
int32_t WaitFor45Degrees();

//******************************************************************************
//	This function takes the closed door reading of the magnetometer and
//	stores in flash
//
//	Description: This function:
//		1.	Take a few reading from magnetometer
//		2.	Average them to smooth out noise
//		3.	Write this to Flash
//******************************************************************************

int32_t Collect_InitMangReadings()
{
	int32_t lRetVal;
	float_t fInitDoorDirectionAngle;
	float_t fMagnFlx_Init[3];
		float_t fMagnFlx_InitAvg[3] = {0,0,0};
		//float_t fMFluxMagnitudeInit = 0;
		uint8_t j;

	//
	// Collect the Initial Magnetometr Readings(door closed)
	//
	UART_PRINT("\n\rMAGNETOMETER:\n\r");
	verifyAccelMagnSensor();
	configureFXOS8700(MODE_READ_ACCEL_MAGN_DATA);
	//DBG

	getDoorDirection( &fInitDoorDirectionAngle );
	DBG_PRINT("DBG: Testing Magnetometer working:\n"\
				" Angle: %f\n\r", fInitDoorDirectionAngle);


	for (j = 0; j < L_AVG_INIT_MAGN; j++)
	{
		getMagnFlux_3axis(fMagnFlx_Init);

		fMagnFlx_InitAvg[0] += fMagnFlx_Init[0];
		fMagnFlx_InitAvg[1] += fMagnFlx_Init[1];
		fMagnFlx_InitAvg[2] += fMagnFlx_Init[2];
//		fMFluxMagnitudeInit += sqrtf( (fMagnFlx_Init[0]*fMagnFlx_Init[0]) +
//											(fMagnFlx_Init[1]*fMagnFlx_Init[1]) +
//											(fMagnFlx_Init[2]*fMagnFlx_Init[2]) );
	}
	standby_accelMagn_fxos8700();
	//fMFluxMagnitudeInit /= L_AVG_INIT_MAGN;
	fMagnFlx_InitAvg[0] /= L_AVG_INIT_MAGN;
	fMagnFlx_InitAvg[1] /= L_AVG_INIT_MAGN;
	fMagnFlx_InitAvg[2] /= L_AVG_INIT_MAGN;

	//
	// Save initial magnetometer readings in flash
	//
	lRetVal = sl_Start(0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = CreateFile_Flash((uint8_t*)MAGN_INIT_VALS_FILE_NAME,
								MAGN_INIT_VALS_FILE_MAXSIZE);
	int32_t lFileHandle;
	lRetVal = WriteFile_ToFlash((uint8_t*)fMagnFlx_InitAvg,
								(uint8_t*)MAGN_INIT_VALS_FILE_NAME, 12, 0,
								SINGLE_WRITE, &lFileHandle);

	lRetVal = sl_Stop(0xFFFF);
	//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//	This function exits when 40degrees_while_closing condition is detected
//
//	The logic used to detect '40 degrees while door is closing':
//		Opening of door beyond 45 degrees is detected. After that, we check
//	when the door angle becomes less than 40 degrees
//
//	Description: This function:
//		1. Reads the door closed position magnetometer reading from Flash
//		2. Poll magnetometer readings, make calcs and wait till door position
//																	> 45 degrees
//		3. Poll magnetometer readings, make calcs and wait till door position
//																	< 44 degrees
//		4. Return
//
//	NOTE: Collect_InitMangReadings() should have been called earlier
//	Moving average filter implemented to smooth out magnetometer noise
//******************************************************************************
int32_t WaitFor40Degrees()
{
	float_t fMagnFluxInit3Axis_Read[3];

	int32_t lRetVal;
	float_t fMFluxMagnitudeInit;
	float_t fThreshold_magnitudeOfDiff, fThreshold_higher_magnitudeOfDiff;
	float_t fMagnFlx_Now[3];
	float_t fMagnFlx_Dif[3];
	float_t fMFluxMagnitudeArray[MOVING_AVG_FLTR_L];
	float_t fMFluxMagnitude;
	uint8_t i = 0, j;
	uint8_t ucOpenFlag = 0;
	uint8_t ucFirstTimeFlag = 1;
	float_t fAngle;
	//
	// Read initial magnetometer readings from flash
	//
	lRetVal = sl_Start(0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = ReadFile_FromFlash((uint8_t*)fMagnFluxInit3Axis_Read,
									(uint8_t*)MAGN_INIT_VALS_FILE_NAME, 12, 0 );

	lRetVal = sl_Stop(0xFFFF);
	//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	//
	// Calculate magnitude corresponding to 40 and 45 degrees
	//

	fMFluxMagnitudeInit = sqrtf( (fMagnFluxInit3Axis_Read[0]*fMagnFluxInit3Axis_Read[0]) +
								(fMagnFluxInit3Axis_Read[1]*fMagnFluxInit3Axis_Read[1]) +
								(fMagnFluxInit3Axis_Read[2]*fMagnFluxInit3Axis_Read[2]) );
	UART_PRINT("DBG: Initial Magnitude: %f\n\r", fMFluxMagnitudeInit);

	fThreshold_magnitudeOfDiff = 2 * fMFluxMagnitudeInit * sinf( (DOOR_ANGLE_TO_DETECT/2) * PI / 180 );
//		fThreshold_magnitudeOfDiff *= fThreshold_magnitudeOfDiff;
	fThreshold_higher_magnitudeOfDiff = 2 * fMFluxMagnitudeInit * sinf( (DOOR_ANGLE_HIGHER/2) * PI / 180);
//		fThreshold_higher_magnitudeOfDiff *= fThreshold_higher_magnitudeOfDiff;
	UART_PRINT("DBG: Threshold Magnitude of Difference: %f   %f\n\r", fThreshold_magnitudeOfDiff, fThreshold_higher_magnitudeOfDiff);

	//
	// Configure sensor for reading accelerometer/magnetometer
	//
	configureFXOS8700(MODE_READ_ACCEL_MAGN_DATA);

	//
	// Wait for '40 degrees while closing condition'
	//

	memset(fMFluxMagnitudeArray, '0', MOVING_AVG_FLTR_L * sizeof(float_t));

	while(1)
	{
//		InitializeTimer();
//		StartTimer();

		getMagnFlux_3axis(fMagnFlx_Now);

		//UART_PRINT("%f,%f,%f\n\r", fMagnFlx_Now[0], fMagnFlx_Now[1], fMagnFlx_Now[2]);

		fMagnFlx_Dif[0] = fMagnFluxInit3Axis_Read[0] - fMagnFlx_Now[0];
		fMagnFlx_Dif[1] = fMagnFluxInit3Axis_Read[1] - fMagnFlx_Now[1];
		fMagnFlx_Dif[2] = fMagnFluxInit3Axis_Read[2] - fMagnFlx_Now[2];
		fMFluxMagnitudeArray[i++] = sqrtf( (fMagnFlx_Dif[0] * fMagnFlx_Dif[0]) +
									(fMagnFlx_Dif[1] * fMagnFlx_Dif[1]) +
									(fMagnFlx_Dif[2] * fMagnFlx_Dif[2]) );
//			fMFluxMagnitudeArray[i++] = ( (fMagnFlx_Dif[0] * fMagnFlx_Dif[0]) +
//										(fMagnFlx_Dif[1] * fMagnFlx_Dif[1]) +
//										(fMagnFlx_Dif[2] * fMagnFlx_Dif[2]) );
		//UART_PRINT("%f\n\r", fMFluxMagnitudeArray[i-1]);
		i = i % MOVING_AVG_FLTR_L;
		//UART_PRINT("%f\n\r", fMFluxMagnitudeInit);

		// For checking magnmtr working only. Comment off otherwise
		/*float_t fDoorDirectionAngle;
		getDoorDirection( &fDoorDirectionAngle );*/

		fMFluxMagnitude = 0;
		for (j = 0; j < MOVING_AVG_FLTR_L; j++)
		{
			fMFluxMagnitude += fMFluxMagnitudeArray[j];
		}
		if( ucFirstTimeFlag == 1 )
		{
			if (i == 0)
			{
				fMFluxMagnitude /= MOVING_AVG_FLTR_L - 1;
				ucFirstTimeFlag = 0;
			}
			else
			{
				fMFluxMagnitude /= (i);
			}
		}
		else
		{
			fMFluxMagnitude /= MOVING_AVG_FLTR_L;
		}

		//DBG. Comment off

		fAngle = 2 * asinf((fMFluxMagnitude/2)/fMFluxMagnitudeInit) *180/PI;
		//UART_PRINT("%f, ", fMFluxMagnitude);
		UART_PRINT("%f,", fAngle);

		if( ucOpenFlag == 0 )
		{
			UART_PRINT("0");
			if( fMFluxMagnitude > fThreshold_higher_magnitudeOfDiff )
			{
				ucOpenFlag = 1;
				UART_PRINT("Open Detected");
			}
		}
		else
		{
			//UART_PRINT("@");
			if( fMFluxMagnitude < fThreshold_magnitudeOfDiff )
			{
				UART_PRINT("Door at 40 degrees while closing");
				//UART_PRINT("\n Angle =  %f\nMagnitude = %f\n",fAngle,fMFluxMagnitude);
//					ucOpenFlag = 0; //To repeat DGB
				break;
			}
		}
//		StopTimer();
//		float_t fTestDuration;
//		GetTimeDuration(&fTestDuration);
//		UART_PRINT("\n\r%f ms\n\r", fTestDuration);

		//Wait for 1.5ms
		//UtilsDelay(.0025*80000000/(3*2));

		UtilsDelay(80000000/(3*2));
	}

	return lRetVal;
}
