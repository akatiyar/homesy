////////////////////////////////////////////////////////////////////////////////
 /*	fxos8700.c
 *	Company Name : Soliton Technologies
 *	Description  : Accelerometer + Magnetometer chip APIs
 *	Date		 :
 *	Version		 :
 *
 *
 *
 *
 */
 ///////////////////////////////////////////////////////////////////////////////
#include "fxos8700.h"

//******************************************************************************
//
//  This function does configures FXOS8700 registers as needed by the
//	applicaiton blueprint/mode
//
//	\param[in]	ucAppMode - Unique value that determines the Application mode
//
//	\return none
//
////******************************************************************************
void configureFXOS8700(uint8_t ucAppMode)
{
	uint8_t i;
	//uint8_t ucConfigArray[] = {0x2A, 0x00, 0x58, 0x01, 0x2A, 0x09};
	uint8_t ucConfigArray[] = {CTRL_REG1, 0x00,
								M_CTRL_REG1, 0x1F,
								M_CTRL_REG2, 0x20,
								XYZ_DATA_CFG, 0x01,
								CTRL_REG1, 0x0D};

	for (i = 0; i < (sizeof(ucConfigArray))/2; i++)
	{
		setConfigReg(ucConfigArray[i*2], ucConfigArray[i*2 + 1]);
//		updateConfigReg(ucConfigArray[i*2],
//							ucConfigArray[i*2 + 1],
//							0b11111111);
	}

//{
//	switch ucAppMode
//	{
//		case MODE2:
//			//writeConfigReg(
//			break;
//	}

	return;
}


//******************************************************************************
//
//  This function checks I2C communication with FXOS8700 by reading the Device
//	ID which is set to 0xC7 for production devices and verifying the same.
//	OUTPUT: UART message - wither success or fialure
//
//	\param none
//
//	\return none
//
//******************************************************************************
#ifdef DEBUG_ENABLE
void checkFXOSDeviceAndI2CCommunication()
{
	uint8_t ucDeviceID;

	i2cReadRegisters(FXOS8700_I2C_ADDRESS,
						DEVICE_ID_REG,
						LENGTH_IS_ONE,
						&ucDeviceID);
	if(DEVICE_ID == ucDeviceID)
	{
		DEBG_PRINT("\nDevice ID read thru' I2C = expected device ID of FXOS8700"
					"\nI2C communication with FXOS8700 SUCCESS\n");
	}
	else
	{
		DEBG_PRINT("\nI2C communication with FXOS8700 FAILED\n CHECK\n");
	}
	return;
}
#endif


//******************************************************************************
//
//	This function sets the Acceleration Threshold value crossing which Motion
//	Detect Interrupt is triggered
//
//	\params[in]	fAccelThreshold - acceleration threshold value (g m/(sec^2));
//									Range: -4 to +4
//									Approximation will be done incase th value
//									is not a multiple of the step value (0.063)
//	\return SUCCESS - if successful
//			FAILURE	- otherwise
//
//******************************************************************************
int32_t setMotionDetectionThreshold(float_t fAccelThreshold)
{

	if((fAccelThreshold >= ACCEL_THRESHOLD_LOWER_LIMIT) &&\
			(fAccelThreshold <= ACCEL_THRESHOLD_UPPER_LIMIT))
	{
		uint8_t iThresholdRegVal;

		iThresholdRegVal = ( ( fAccelThreshold - ACCEL_THRESHOLD_LOWER_LIMIT )\
									/ ACCEL_THRESHOLD_STEP_VAL );

		updateConfigReg(ACCEL_THRSLD_REG,
							iThresholdRegVal,
							MASK_BIT0TO6);
	}
	else
	{
		return FAILURE;
	}

	return SUCCESS;
}

//******************************************************************************
//
//  This function update the value of a single register, particularly for
//	configuring the device, in FXOS8700
//
//	\param[in]	ucConfigRegAddr - Address of a configuration register, held in a
//									macro
//	\param[in]	ucConfigVal - Configuration Value to be updated or ORed to the
//									register, can be a macro or variable
//	\param[in]	ucBitMask - 8-bit mask with value 1 on positions to be updated
//
//	\return none
//
//******************************************************************************
void updateConfigReg(uint8_t ucConfigRegAddr,
						uint8_t ucConfigVal,
						uint8_t ucBitMask)
{
	uint8_t ucReadRegVal;

	i2cReadRegisters(FXOS8700_I2C_ADDRESS,
						ucConfigRegAddr,
						LENGTH_IS_ONE,
						&ucReadRegVal);

	ucReadRegVal &= ~ucBitMask;
	ucReadRegVal |= ucConfigVal;

	i2cWriteRegisters(FXOS8700_I2C_ADDRESS,
						ucConfigRegAddr,
						LENGTH_IS_ONE,
						&ucConfigVal);

	return;
}


//******************************************************************************
//
//	This function gives the direction of the door with respect to geographic
//	north from FXOS8700
//
//	\param[out] pfDegrees - pointer to the variable where door direction
//									is stored. Door direction unit is 'degrees
//									from north'
//
//	\return none
//
//******************************************************************************
void getDoorDirection(float_t* pfDegrees)
{
	uint8_t ucMagntOut[LENGTH_OUTPUT_DATA];
	uint16_t usMagntData_ParallelAxis, usMagntData_PerpendicularAxis;
	float_t fTesla_ParallelAxis, fTesla_PerpendicularAxis;
	float_t fYawAngleVal;

	//
	//	Get M_OUT_X_MSB, M_OUT_X_LSB, M_OUT_Y_MSB, etc from FXOS8700
	//
	getMagntData(ucMagntOut);
	DEBG_PRINT("\n\nMagn Data Read: %x %x %x %x %x %x",
								ucMagntOut[0],
								ucMagntOut[1],
								ucMagntOut[2],
								ucMagntOut[3],
								ucMagntOut[4],
								ucMagntOut[5]);

	//
	//	Calculate Flux Density(uT) from the raw output data for ll and T axis
	//
	usMagntData_ParallelAxis = ( (ucMagntOut[PARALLEL_AXIS*2] << 8) +
										ucMagntOut[PARALLEL_AXIS*2 + 1] );
	fTesla_ParallelAxis = usMagntData_ParallelAxis / SENSITIVITY_MAG;

	usMagntData_PerpendicularAxis = ( (ucMagntOut[PERPENDICULAR_AXIS*2] << 8) +
										ucMagntOut[PERPENDICULAR_AXIS*2 + 1] );
	fTesla_PerpendicularAxis = usMagntData_PerpendicularAxis / SENSITIVITY_MAG;

	//
	//	Find the degrees from north the parallel axis (= door direction) is
	//
	fYawAngleVal = ( atan2l(fTesla_PerpendicularAxis, fTesla_ParallelAxis)
						* 180 / PI );
	*pfDegrees = (-fYawAngleVal);

	return;
}


//******************************************************************************
//
//	This function gives the linear acceleration value (in units of g m/(sec^2))
//
//	\param[out]	pfAcceleration - pointer to the variable where linear
//									acceleration considering the horizontal
//									axes only
//
//	\return none
//
//******************************************************************************
