////////////////////////////////////////////////////////////////////////////////
 /*	accelomtrMagntomtr_fxos8700.c
 *	Company Name : Soliton Technologies
 *	Description  : Accelerometer + Magnetometer chip APIs
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////
#include <accelomtr_magntomtr_fxos8700.h>
#include "flash_files.h"

#define HEADER_SIZE						3

#define DEVICE_ID_REG 					0x0D
#define DEVICE_ID 						0xC7

#define FLASH_CONFIGS					0

//******************************************************************************
//
//  This function checks I2C communication with FXOS8700 by reading the Device
//	ID which is set to 0xC7 for production devices and verifying the same.
//
//	\param none
//
//	\return 0 on success and FXOS8700_NOT_FOUND on failure
//
//******************************************************************************
int32_t verifyAccelMagnSensor()
{
	uint8_t ucDeviceID = 0x00;

	int32_t lRetVal;

	lRetVal = i2cReadRegisters(FXOS8700_I2C_ADDRESS,
						DEVICE_ID_REG,
						LENGTH_IS_ONE,
						&ucDeviceID);
	PRINT_ON_ERROR(lRetVal);

	DEBG_PRINT("A+M Device ID: %x\n", ucDeviceID);

	if(DEVICE_ID == ucDeviceID)
	{
		DEBG_PRINT("I2C comm. with FXOS8700 SUCCESS\n");
		lRetVal = SUCCESS;
	}
	else
	{
		DEBG_PRINT("FXOS8700 I2C comm. FAILED!\n");
		lRetVal = FXOS8700_NOT_FOUND;
	}

	return FXOS8700_NOT_FOUND;
}

//******************************************************************************
// This function puts FXOS8700 in standby.
//
// During standby,
//	Most register values are retained. Some register vals are lost.
//	I2C communications can happen during standby.
//	Typical Standby current = 2uA
//******************************************************************************
int32_t standby_accelMagn_fxos8700()
{
	uint8_t data = 0x00;
	int32_t lRetVal;

	lRetVal = setConfigReg(CTRL_REG1, data);
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//
//  This function fetches current magnetometer readings of all the 3 axis and
//	accerlerometer readings of all the 3 axis from FXOS8700
//
//	\param[out] pucData - pointer to array to which magnetometer and
//							accelerometer data are stored
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
int32_t getAccelMagntData(uint8_t* pucAccelMagntData)
{
	int32_t lRetVal;

	lRetVal = i2cReadRegisters(FXOS8700_I2C_ADDRESS, ACCEL_OUTPUT_DATA_REG,
									LENGTH_OUTPUT_DATA*2, pucAccelMagntData);
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
// This function does a software reset of FXOS
//
// Can be called in active / standby mode of FXOS
//******************************************************************************
int32_t reset_accelMagn_fxos8700()
{
	uint8_t data = 0x40;
	int32_t lRetVal;
	uint8_t ucTryNum;

	DEBG_PRINT("Reset AM\n");

	// This function call returns error, since FXOS does not give
	//acknowledgement when reset is activated
	lRetVal = setConfigReg(CTRL_REG2, data);
	//PRINT_ON_ERROR(lRetVal);

	UtilsDelay((ONESEC_NO_OF_CYCLES/1000 + 100));	//give a one milli sec delay
													//before any other operation
													//with FXOS

	//	Wait till CTRL_REG2 is equal to 0x00
	ucTryNum = 1;
	while((data != 0x00) && (ucTryNum <= 5))
	{
		lRetVal = i2cReadRegisters(FXOS8700_I2C_ADDRESS, CTRL_REG2, 1, &data);
		PRINT_ON_ERROR(lRetVal);
		DEBG_PRINT("0x00:%x\n",data);
		UtilsDelay(10000);
		ucTryNum++;
	}

	if(data != 0x00)
	{
		return UNABLE_TO_RESET_FXOS8700;
	}

	return SUCCESS;
}

#if 0
//******************************************************************************
//
//  This function fetches current magnetometer readings of all the 3 axis from
//	FXOS8700 by calling i2cReadRegister API
//
//	\param[out] pucData - pointer to array to which magnetometer data is
//								stored
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
int32_t getMagntData(uint8_t* pucMagntData)
{
	int32_t lRetVal;

	lRetVal = i2cReadRegisters(FXOS8700_I2C_ADDRESS, MAGNT_OUTPUT_DATA_REG,
									LENGTH_OUTPUT_DATA, pucMagntData);
	PRINT_ON_ERROR(lRetVal);
	return lRetVal;
}

//******************************************************************************
//
//	This function sets the Acceleration Threshold value crossing which Motion
//	Detect Interrupt is triggered
//
//	\params[in]	fAccelThreshold - acceleration threshold value (g m/(sec^2));
//									Range: -4 to +4
//									Approximation will be done incase th value
//									is not a multiple of the step value (0.063)
//	\return none
//
//******************************************************************************
void setMotionDetectionThreshold(float_t fAccelThreshold)
{

	uint8_t iThresholdRegVal;

	if((fAccelThreshold >= ACCEL_THRESHOLD_LOWER_LIMIT) &&\
			(fAccelThreshold <= ACCEL_THRESHOLD_UPPER_LIMIT))
	{
		iThresholdRegVal = ( ( fAccelThreshold - ACCEL_THRESHOLD_LOWER_LIMIT )\
									/ ACCEL_THRESHOLD_STEP_VAL );

		updateConfigReg(ACCEL_THRSLD_REG,
							iThresholdRegVal,
							MASK_BIT0TO6);
	}
	else
	{
		DEBG_PRINT("\nGiven Threshold not within range\n");
	}
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
	int16_t sMagData_llAxis, sMagData_TAxis;
	float_t fYawAngleVal;

	//
	//	Get M_OUT_X_MSB, M_OUT_X_LSB, M_OUT_Y_MSB, etc from FXOS8700
	//
	getMagntData(ucMagntOut);

	//
	//	Calculate Flux Density(uT) from the raw output data for ll and T axis
	//
	sMagData_llAxis = (int16_t)( (ucMagntOut[PARALLEL_AXIS*2] << 8) +
										ucMagntOut[PARALLEL_AXIS*2 + 1] );
	sMagData_TAxis = (int16_t)( (ucMagntOut[PERPENDICULAR_AXIS*2] << 8) +
										ucMagntOut[PERPENDICULAR_AXIS*2 + 1] );

	//
	//	Find the degrees from the North the parallel axis (= door direction) is
	//
	fYawAngleVal = ( atan2l(sMagData_TAxis, sMagData_llAxis) * 180 / PI );

	//fYawAngleVal = ( atan2l(-sMagData_TAxis, sMagData_llAxis) * 180 / PI );
									// The minus in the formula is put there
									// since the chip is soldered upside down
//	*pfDegrees = (-fYawAngleVal); 	// The above formula gives degrees from door
//									// to the North

//	*pfDegrees = (fYawAngleVal);	// Returning the door angle from north for
//									// intermediate prototype testing

	if( (fYawAngleVal) >= 0 )
		*pfDegrees = (fYawAngleVal);	// Door's angle from north (0 to 360)
										// in anti-clockwise direction
	else
		*pfDegrees = 360 + (fYawAngleVal);

	return;
}

void getMagnFlux_3axis(float_t* pfMagnFlux_3axis)
{
	uint8_t ucMagntOut[LENGTH_OUTPUT_DATA];

	//
	//	Get M_OUT_X_MSB, M_OUT_X_LSB, M_OUT_Y_MSB, etc from FXOS8700
	//
	getMagntData(ucMagntOut);

	//
	//	Calculate Flux Density(uT) from the raw output data for ll and T axis
	//
	*pfMagnFlux_3axis = (int16_t)( (ucMagntOut[0] << 8) + ucMagntOut[1] );
	*pfMagnFlux_3axis = (*pfMagnFlux_3axis) * SENSITIVITY_MAG;
	pfMagnFlux_3axis++;
	*pfMagnFlux_3axis = (int16_t)( (ucMagntOut[2] << 8) + ucMagntOut[3] );
	*pfMagnFlux_3axis = (*pfMagnFlux_3axis) * SENSITIVITY_MAG;
	pfMagnFlux_3axis++;
	*pfMagnFlux_3axis = (int16_t)( (ucMagntOut[4] << 8) + ucMagntOut[5] );
	*pfMagnFlux_3axis = (*pfMagnFlux_3axis) * SENSITIVITY_MAG;

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
void getAccelerationMagnitude(float_t* pfAccel)
{
	uint8_t ucAccelOut[6];

	getAccelData(&ucAccelOut[0]);
//	DEBG_PRINT("\nAcc Data Read: %x %x %x %x %x %x\n\r",
//					ucAccelOut[0],
//					ucAccelOut[1],
//					ucAccelOut[2],
//					ucAccelOut[3],
//					ucAccelOut[4],
//					ucAccelOut[5]);
//	DEBG_PRINT("x: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2)));
//	DEBG_PRINT("y: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2)));
//	DEBG_PRINT("z: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2)));


	DEBG_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2)));
	DEBG_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2)));
	DEBG_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2)));

	*pfAccel = pow( (int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2, 2);
	*pfAccel +=  pow( (int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2, 2);
	*pfAccel +=  pow( (int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2, 2);
	*pfAccel = sqrt(*pfAccel) * SENSITIVITY_ACCEL;

	DEBG_PRINT( "%f\n", *pfAccel);

	return;
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
}

//******************************************************************************
//
//	This function clears the accelerometer motion iterrupt by reading the
//	Accelerometer Freefall/motion detection interrupt source register
//
//	param	none
//
//	return	none
//
//******************************************************************************
void clearAccelMotionIntrpt()
{
	uint8_t ucMotionIntrptStatus;

	i2cReadRegisters(FXOS8700_I2C_ADDRESS,
						A_FFMT_SRC_REG,
						LENGTH_IS_ONE,
						&ucMotionIntrptStatus);
	DEBG_PRINT("\n\rMotion Detect Intrpt Status: %x\n\r", ucMotionIntrptStatus);

	return;
}

//	Calibration Related Function taken from https://community.freescale.com/docs/DOC-101073
//	Called by the FXOS8700CQ_Mag_Calibration()
void FXOS8700CQ_Init (void)
{
	uint8_t data;

	data = 0x40;
	setConfigReg(CTRL_REG2, data);          // Reset all registers to POR values

	UtilsDelay(.001*8000000/3);        // ~1ms delay

    data = 0x00;
    setConfigReg(XYZ_DATA_CFG_REG, data);   // +/-2g range with 0.244mg/LSB

    data = 0x1F;
    setConfigReg(M_CTRL_REG1, data);        // Hybrid mode (accelerometer + magnetometer), max OSR
    data = 0x20;
    setConfigReg(M_CTRL_REG2, data);        // M_OUT_X_MSB register 0x33 follows the OUT_Z_LSB register 0x06 (used for burst read)

    data = 0x02;
    setConfigReg(CTRL_REG2, data);          // High Resolution mode
    data = 0x02;
    setConfigReg(CTRL_REG3, data);          // Push-pull, active high interrupt
    data = 0x01;
    setConfigReg(CTRL_REG4, data);          // Enable DRDY interrupt
    data = 0x01;
    setConfigReg(CTRL_REG5, data);          // DRDY interrupt routed to INT1 - PTD4
    data = 0x35;
    setConfigReg(CTRL_REG1, data);          // ODR = 3.125Hz, Reduced noise, Active mode
}

//	Calibration Function taken from https://community.freescale.com/docs/DOC-101073
void FXOS8700CQ_Mag_Calibration (void)
{
	 short Xout_Mag_16_bit, Yout_Mag_16_bit, Zout_Mag_16_bit;
     short Xout_Mag_16_bit_avg, Yout_Mag_16_bit_avg, Zout_Mag_16_bit_avg;
     short Xout_Mag_16_bit_max, Yout_Mag_16_bit_max, Zout_Mag_16_bit_max;
     short Xout_Mag_16_bit_min, Yout_Mag_16_bit_min, Zout_Mag_16_bit_min;
     char i = 0;
     uint8_t AccelMagData[6];
     uint8_t data;

     FXOS8700CQ_Init();

     while (i < 94)             // This takes ~30s (94 samples * 1/3.125)
     {

    	 //while(!GPIOPinRead(GPIOA0_BASE,0x80))
    	 while(!GPIOPinRead(GPIOA0_BASE,0x04))
    	 {
    	 }

    	DEBG_PRINT("!");
        {
             //i2cReadRegisters(FXOS8700_I2C_ADDRESS, MOUT_X_MSB_REG, 6, AccelMagData);         // Read data output registers 0x33 - 0x38
             getMagntData(AccelMagData);
             DEBG_PRINT("\nMagn Data Read: %x %x %x %x %x %x",
             								AccelMagData[0],
             								AccelMagData[1],
             								AccelMagData[2],
             								AccelMagData[3],
             								AccelMagData[4],
             								AccelMagData[5]);

             Xout_Mag_16_bit = (short) (AccelMagData[0]<<8 | AccelMagData[1]);        // Compute 16-bit X-axis magnetic output value
             Yout_Mag_16_bit = (short) (AccelMagData[2]<<8 | AccelMagData[3]);        // Compute 16-bit Y-axis magnetic output value
             Zout_Mag_16_bit = (short) (AccelMagData[4]<<8 | AccelMagData[5]);        // Compute 16-bit Z-axis magnetic output value

             // Assign first sample to maximum and minimum values
             if (i == 0)
             {
                  Xout_Mag_16_bit_max = Xout_Mag_16_bit;
                  Xout_Mag_16_bit_min = Xout_Mag_16_bit;

                  Yout_Mag_16_bit_max = Yout_Mag_16_bit;
                  Yout_Mag_16_bit_min = Yout_Mag_16_bit;

                  Zout_Mag_16_bit_max = Zout_Mag_16_bit;
                  Zout_Mag_16_bit_min = Zout_Mag_16_bit;
             }

			  // Check to see if current sample is the maximum or minimum X-axis value
			  if (Xout_Mag_16_bit > Xout_Mag_16_bit_max)    {Xout_Mag_16_bit_max = Xout_Mag_16_bit;}
			  if (Xout_Mag_16_bit < Xout_Mag_16_bit_min)    {Xout_Mag_16_bit_min = Xout_Mag_16_bit;}

			  // Check to see if current sample is the maximum or minimum Y-axis value
			  if (Yout_Mag_16_bit > Yout_Mag_16_bit_max)    {Yout_Mag_16_bit_max = Yout_Mag_16_bit;}
			  if (Yout_Mag_16_bit < Yout_Mag_16_bit_min)    {Yout_Mag_16_bit_min = Yout_Mag_16_bit;}

			  // Check to see if current sample is the maximum or minimum Z-axis value
			  if (Zout_Mag_16_bit > Zout_Mag_16_bit_max)    {Zout_Mag_16_bit_max = Zout_Mag_16_bit;}
			  if (Zout_Mag_16_bit < Zout_Mag_16_bit_min)    {Zout_Mag_16_bit_min = Zout_Mag_16_bit;}

             i++;
         }

     }

     Xout_Mag_16_bit_avg = (Xout_Mag_16_bit_max + Xout_Mag_16_bit_min) / 2;            // X-axis hard-iron offset
     Yout_Mag_16_bit_avg = (Yout_Mag_16_bit_max + Yout_Mag_16_bit_min) / 2;            // Y-axis hard-iron offset
     Zout_Mag_16_bit_avg = (Zout_Mag_16_bit_max + Zout_Mag_16_bit_min) / 2;            // Z-axis hard-iron offset
     DEBG_PRINT("\nCalibrated Offset Val: %x, %x, %x\nWrite one bit left shifted value into register",
    		 	 	 	 	 Xout_Mag_16_bit_avg, Yout_Mag_16_bit_avg, Zout_Mag_16_bit_avg);


     // Left-shift by one as magnetometer offset registers are 15-bit only, left justified
     Xout_Mag_16_bit_avg <<= 1;
     Yout_Mag_16_bit_avg <<= 1;
     Zout_Mag_16_bit_avg <<= 1;

     data = 0x00;
     setConfigReg( CTRL_REG1, data);          // Standby mode to allow writing to the offset registers

     data = (char) (Xout_Mag_16_bit_avg & 0xFF);
     setConfigReg( MOFF_X_LSB_REG, data );
     data = (char) ((Xout_Mag_16_bit_avg >> 8) & 0xFF);
     setConfigReg( MOFF_X_MSB_REG, data );
     data = (char) (Yout_Mag_16_bit_avg & 0xFF);
     setConfigReg( MOFF_Y_LSB_REG, data );
     data = (char) ((Yout_Mag_16_bit_avg >> 8) & 0xFF);
     setConfigReg( MOFF_Y_MSB_REG, data);
     data = (char) (Zout_Mag_16_bit_avg & 0xFF);
     setConfigReg( MOFF_Z_LSB_REG, data);
     data = (char) ((Zout_Mag_16_bit_avg >> 8) & 0xFF);
     setConfigReg( MOFF_Z_MSB_REG, data );

     data = 0x35;
     setConfigReg(CTRL_REG1, data);          // Active mode again
}

//******************************************************************************
//
//  A ROUGH UNTESTED function to write magnetometer calibration values
//	(determined from previous calibration) into corresponding registers
//	in FXOS8700
//
//	\param[in]	psCalibOffsetVals - Pointer to array that holds predetermined
//										magnetometer calibration values
//
//	\return none
//
//******************************************************************************
void writeMagntCalibrationValue(int16_t* psCalibOffsetVals)
{
	uint8_t ucCalibOffsetVals[6];

	ucCalibOffsetVals[0] = (char) ( ( (*psCalibOffsetVals) >> 8) & 0xFF);
	ucCalibOffsetVals[1] = (char) ((*psCalibOffsetVals) & 0xFF);
	psCalibOffsetVals++;
	ucCalibOffsetVals[2] = (char) ( ( (*psCalibOffsetVals) >> 8) & 0xFF);
	ucCalibOffsetVals[3] = (char) ((*psCalibOffsetVals) & 0xFF);
	psCalibOffsetVals++;
	ucCalibOffsetVals[4] = (char) ( ( (*psCalibOffsetVals) >> 8) & 0xFF);
	ucCalibOffsetVals[5] = (char) ((*psCalibOffsetVals) & 0xFF);

	i2cWriteRegisters(FXOS8700_I2C_ADDRESS,
						MOFF_X_MSB_REG,
						LENGTH_IS_SIX,
						ucCalibOffsetVals);
}

//******************************************************************************
//
//  This function configures FXOS8700 registers as needed by the applicaiton
//	blueprint/mode
//
//	\param[in]	ucAppMode - Unique value that determines the Application mode
//
//	\return none
//
//******************************************************************************
void configureFXOS8700(uint8_t ucAccelMagnMode)
{
	uint8_t i = 0;
	uint8_t ucConfigArray[100];
	uint8_t ucNoofCfgs;
	uint8_t ucHeader[HEADER_SIZE];
	int32_t lRetVal;

	switch(ucAccelMagnMode)
	{
		case MODE_READ_ACCEL_MAGN_DATA:
			if(FLASH_CONFIGS)
			{
				//Read file from flash into ucConfig
				ReadFile_FromFlash(ucHeader,
									FILENAME_SENSORCONFIGS,
									HEADER_SIZE,
									OFFSET_FXOS8700);
				ReadFile_FromFlash(ucConfigArray,
									FILENAME_SENSORCONFIGS,
									(uint8_t)ucHeader[1],
									OFFSET_FXOS8700+HEADER_SIZE);
				ucNoofCfgs = ucHeader[1]/2;
				break;
			}
			// Software reset
			ucConfigArray[i++] = CTRL_REG2;
			ucConfigArray[i++] = 0x40;

			// Stand by mode
			ucConfigArray[i++] = CTRL_REG1;
			ucConfigArray[i++] = 0x00;

			// Hybrid(A+M), M_OSR=7;
			// OTHER OPTS: 0x1D-Magnetometer only; 0x03-Magnetometer OSR = 0
			ucConfigArray[i++] = M_CTRL_REG1;
			ucConfigArray[i++] = 0x1F;

			// Hyb Auto Inc Mode - not needed!
			ucConfigArray[i++] = M_CTRL_REG2;
			ucConfigArray[i++] = 0x20;

			// -2 to +2 full scale	// 0x01 - -4 to +4 FS
			ucConfigArray[i++] = XYZ_DATA_CFG_REG;
			ucConfigArray[i++] = 0x00;

			// High resolution mode
			ucConfigArray[i++] = CTRL_REG2;
			ucConfigArray[i++] = 0x02;

			ucConfigArray[i++] = CTRL_REG3;
			ucConfigArray[i++] = 0x02;
			ucConfigArray[i++] = CTRL_REG4;
			ucConfigArray[i++] = 0x01;
			ucConfigArray[i++] = CTRL_REG5;
			ucConfigArray[i++] = 0x01;

			// Stand by mode
			ucConfigArray[i++] = CTRL_REG1;
			ucConfigArray[i++] = 0x00;

			// Magnetometer Calibration values hard coded
//			ucConfigArray[i++] = MOFF_X_MSB_REG;
//			ucConfigArray[i++] = 0x05;
//			ucConfigArray[i++] = MOFF_X_LSB_REG;
//			ucConfigArray[i++] = 0x80;
//			ucConfigArray[i++] = MOFF_Y_MSB_REG;
//			ucConfigArray[i++] = 0x02;
//			ucConfigArray[i++] = MOFF_Y_LSB_REG;
//			ucConfigArray[i++] = 0x7C;
//			ucConfigArray[i++] = MOFF_Z_MSB_REG;
//			ucConfigArray[i++] = 0x05;
//			ucConfigArray[i++] = MOFF_Z_LSB_REG;
//			ucConfigArray[i++] = 0x00;

			ucConfigArray[i++] = MOFF_X_MSB_REG;	//Hand soldered on PCB
			ucConfigArray[i++] = (0x025f >> 7);
			ucConfigArray[i++] = MOFF_X_LSB_REG;
			ucConfigArray[i++] = ((0x025f << 1) & 0x00ff);
			ucConfigArray[i++] = MOFF_Y_MSB_REG;
			ucConfigArray[i++] = (0xFE91 >> 7);
			ucConfigArray[i++] = MOFF_Y_LSB_REG;
			ucConfigArray[i++] = ((0xfe91 << 1) & 0x00ff);
			ucConfigArray[i++] = MOFF_Z_MSB_REG;
			ucConfigArray[i++] = (0x350 >> 7);
			ucConfigArray[i++] = MOFF_Z_LSB_REG;
			ucConfigArray[i++] = ((0x350 << 1) & 0x00ff);

			// ODR (hybridmode) = 3.125 Hz
			//0x0D - ; 0x25 - ODR (hybridmode) = 25 Hz
			//0x05 - ODR (hybridmode) = 400 Hz
			ucConfigArray[i++] = CTRL_REG1;
			//ucConfigArray[i++] = 0x35;
			ucConfigArray[i++] = 0x05;

			ucNoofCfgs = i/2;

			break;

		case MODE_ACCEL_INTERRUPT: //For motion detection: Configs from AN4070
			if(FLASH_CONFIGS)
			{
				//Read file from flash into ucConfig
				ReadFile_FromFlash(ucHeader,
									FILENAME_SENSORCONFIGS,
									HEADER_SIZE,
									OFFSET_FXOS8700);
				ReadFile_FromFlash(ucConfigArray,
									FILENAME_SENSORCONFIGS,
									(uint8_t)ucHeader[2],
									OFFSET_FXOS8700+HEADER_SIZE+ucHeader[1]);
				ucNoofCfgs = ucHeader[2]/2;
				break;
			}
			// Software reset
			ucConfigArray[i++] = CTRL_REG2;
			ucConfigArray[i++] = 0x40;

			// Stand by mode; ORD = 100Hz
			ucConfigArray[i++] = CTRL_REG1;
			ucConfigArray[i++] = 0x18;

			// “OR”, latch enable, enabling X, Z (change with chip position)
			// Bit5,6,7 for Z,Y,X respectively
			ucConfigArray[i++] = A_FFMT_CFG_REG;
			ucConfigArray[i++] = 0xE8;

			// Motion detection threshold(common to X,Y,Z), MAX:0x7F, 1LSB=.063
			ucConfigArray[i++] = A_FFMT_THRS_REG;
			ucConfigArray[i++] = 0x02;

			// Debounce Counter
			//Calcs: Req Debnc_Time = 100ms, o/p gap = 1/ODR = 10ms => cnt=10
			ucConfigArray[i++] = A_FFMT_DEBOUNCE_CNT_REG;
			ucConfigArray[i++] = 0x0A;

			// Enable Motion/Freefall Interrupt Function in the System
			ucConfigArray[i++] = CTRL_REG4;
			ucConfigArray[i++] = 0x04;

			// Route the Motion/Freefall Interrupt Function to INT1 hardware pin
			ucConfigArray[i++] = CTRL_REG5;
			ucConfigArray[i++] = 0x04;

			// Active Mode
			ucConfigArray[i++] = CTRL_REG1;
			ucConfigArray[i++] = (0x18|0x01);//Some fields are set earlier
												// Use updateReg fn if feasible

			ucNoofCfgs = i/2;

			break;
	}
	for (i = 0; i < ucNoofCfgs; i++)
	{
		lRetVal = setConfigReg(ucConfigArray[i*2], ucConfigArray[i*2 + 1]);
//		updateConfigReg(ucConfigArray[i*2],
//							ucConfigArray[i*2 + 1],
//							0b11111111);
		PRINT_ON_ERROR(lRetVal);

	}
	UtilsDelay(80000000*.3);

	return;
}

#endif
