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
#include "accelomtrMagntomtr_fxos8700.h"

//******************************************************************************
//
//  This function fetches current accelerometer readings of all the 3 axis from
//	FXOS8700 by calling i2cReadRegister API
//
//	\param[out] pucAccelData - pointer to array to which accelerometer data is
//								stored
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define getAccelData(pucAccelData) i2cReadRegisters(FXOS8700_I2C_ADDRESS,\
									ACCEL_OUTPUT_DATA_REG,\
									LENGTH_OUTPUT_DATA,\
									pucAccelData)

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
#define getMagntData(pucMagntData) i2cReadRegisters(FXOS8700_I2C_ADDRESS,\
									MAGNT_OUTPUT_DATA_REG,\
									LENGTH_OUTPUT_DATA,\
									pucMagntData)

//******************************************************************************
//
//  This function writes the value of a single register, particularly for a
//	status register, in FXOS8700
//
//	\param[in]	ucStatusRegAddr - Address of a configuration register, held in a
//									macro
//	\param[out]	pucStatusVal - Status value as read from the register address
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define readStatusReg(ucStatusRegAddr, pucStatusVal)\
								i2cReadRegisters(FXOS8700_I2C_ADDRESS,\
									ucStatusRegAddr,\
									LENGTH_IS_ONE,\
									pucStatusVal)

//******************************************************************************
//
//  This function writes a value to a single register, particularly for
//	configuring the device, in FXOS8700
//
//	\param[in]	ucConfigRegAddr - Address of a configuration register, held in a
//									macro
//	\param[in]	ucConfigVal -	Configuration Value to be written to the
//									register, can be a macro or variable
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define setConfigReg(ucConfigRegAddr, ucConfigVal)\
								i2cWriteRegisters(FXOS8700_I2C_ADDRESS,\
									ucConfigRegAddr,\
									LENGTH_IS_ONE,\
									&ucConfigVal)

//******************************************************************************
//
//  This function checks I2C communication with FXOS8700 by reading the Device
//	ID which is set to 0xC7 for production devices and verifying the same.
//	OUTPUT: UART message - whether success or fialure
//
//	\param none
//
//	\return none
//
//******************************************************************************
void verifyAccelMagnSensor()
{
	uint8_t ucDeviceID = 0x00;

	i2cReadRegisters(FXOS8700_I2C_ADDRESS,
						DEVICE_ID_REG,
						LENGTH_IS_ONE,
						&ucDeviceID);

	UART_PRINT("Read Device ID: %x", ucDeviceID);

	if(DEVICE_ID == ucDeviceID)
	{
		UART_PRINT("\nDevice ID read thru' I2C = expected device ID of FXOS8700"
					"\nI2C communication with FXOS8700 SUCCESS");
	}
	else
	{
		UART_PRINT("\nFXOS8700 I2C communication FAILED\n CHECK\n");
	}
	return;
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
	uint8_t ucConfigArray[40];
	uint8_t ucNoofCfgs;

	switch(ucAccelMagnMode)
	{
		case MODE_READ_ACCEL_MAGN_DATA:
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
			ucConfigArray[i++] = MOFF_X_MSB_REG;
			ucConfigArray[i++] = 0x05;
			ucConfigArray[i++] = MOFF_X_LSB_REG;
			ucConfigArray[i++] = 0x80;
			ucConfigArray[i++] = MOFF_Y_MSB_REG;
			ucConfigArray[i++] = 0x02;
			ucConfigArray[i++] = MOFF_Y_LSB_REG;
			ucConfigArray[i++] = 0x7C;
			ucConfigArray[i++] = MOFF_Z_MSB_REG;
			ucConfigArray[i++] = 0x05;
			ucConfigArray[i++] = MOFF_Z_LSB_REG;
			ucConfigArray[i++] = 0x00;

			// ODR (hybridmode) = 3.125 Hz
			//0x0D - ; 0x25 - ODR (hybridmode) = 25 Hz
			//0x05 - ODR (hybridmode) = 400 Hz
			ucConfigArray[i++] = CTRL_REG1;
			ucConfigArray[i++] = 0x35;

			ucNoofCfgs = i/2;

			break;

		case MODE_ACCEL_INTERRUPT: //For motion detection: Configs from AN4070
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
		setConfigReg(ucConfigArray[i*2], ucConfigArray[i*2 + 1]);
//		updateConfigReg(ucConfigArray[i*2],
//							ucConfigArray[i*2 + 1],
//							0b11111111);
	}
	return;
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
		UART_PRINT("\nGiven Threshold not within range\n");
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

	*pfDegrees = (fYawAngleVal);	// Returning the door angle from north for
									//	intermediate prototype testing

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
//	UART_PRINT("\nAcc Data Read: %x %x %x %x %x %x\n\r",
//					ucAccelOut[0],
//					ucAccelOut[1],
//					ucAccelOut[2],
//					ucAccelOut[3],
//					ucAccelOut[4],
//					ucAccelOut[5]);
//	UART_PRINT("x: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2)));
//	UART_PRINT("y: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2)));
//	UART_PRINT("z: %f", (SENSITIVITY_ACCEL *
//			(float_t)((int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2)));


	UART_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2)));
	UART_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2)));
	UART_PRINT("%f, ", (SENSITIVITY_ACCEL *
			(float_t)((int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2)));

	*pfAccel = pow( (int16_t)((ucAccelOut[0] << 8) | ucAccelOut[1]) >> 2, 2);
	*pfAccel +=  pow( (int16_t)((ucAccelOut[2] << 8) | ucAccelOut[3]) >> 2, 2);
	*pfAccel +=  pow( (int16_t)((ucAccelOut[4] << 8) | ucAccelOut[5]) >> 2, 2);
	*pfAccel = sqrt(*pfAccel) * SENSITIVITY_ACCEL;

	UART_PRINT( "%f\n", *pfAccel);

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
	UART_PRINT("\n\rMotion Detect Intrpt Status: %x\n\r", ucMotionIntrptStatus);

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

    	UART_PRINT("!");
        {
             //i2cReadRegisters(FXOS8700_I2C_ADDRESS, MOUT_X_MSB_REG, 6, AccelMagData);         // Read data output registers 0x33 - 0x38
             getMagntData(AccelMagData);
             UART_PRINT("\nMagn Data Read: %x %x %x %x %x %x",
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
     UART_PRINT("\nCalibrated Offset Val: %x, %x, %x\nWrite one bit left shifted value into register",
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
