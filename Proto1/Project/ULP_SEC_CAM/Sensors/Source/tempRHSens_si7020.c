////////////////////////////////////////////////////////////////////////////////
 /*	tempRHSens_si7020.c
 *	Company Name : Soliton Technologies
 *	Description  : Temp + RH sensor chip APIs
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////
#include "tempRHSens_si7020.h"

//******************************************************************************
//
//  This function checks I2C communication with Si7020 by reading the Device
//	ID which is set to 0x14 for Si7020 device.
//	OUTPUT: UART message - whether success or fialure
//
//	\param none
//
//	\return none
//
//******************************************************************************
void verifyTempRHSensor()
{
	uint8_t ucDeviceID[4];
	uint8_t ucCommand[] = {FETCH_ID_CMD_BYTE1, FETCH_ID_CMD_BYTE2};

	i2cReadRegistersTwoBytes(SI7020_I2C_ADDRESS,
							ucCommand,
							LENGTH_IS_FOUR,
							ucDeviceID);

	UART_PRINT("\nData Read: %x %x %x %x\n",
						ucDeviceID[0], ucDeviceID[1], ucDeviceID[2], ucDeviceID[3]);
	if(DEVICE_ID == ucDeviceID[0])
	{
		UART_PRINT("\nDevice ID read thru' I2C = expected device ID of Si7020"
					"\nI2C communication with Si7020 SUCCESS\n");
	}
	else
	{
		UART_PRINT("\nSi7020 I2C communication FAILED\nPLEASE CHECK\n");
	}

	return;
}

//******************************************************************************
//
//  This function configures the Si7020 sensor's temperature and RH resolutions
//	and heater state
//
//	\param none
//
//	\return none
//
//******************************************************************************
void configureTempRHSensor()
{
	uint8_t ucResolution = RH8BIT_TEMP12BIT;	// Conversion time - resolution
												// trade-off
	uint8_t ucHeaterEnable = 0;	// Use HEATER_ENABLE to switch ON heater

	uint8_t ucConfigRegVal;
	uint8_t ucBitMask = RESOLUTION_MASK|HEATER_ENABLE_MASK;

	i2cReadRegisters(SI7020_I2C_ADDRESS,
						READ_CONFIGREG_CMD, //Read Command instead of RegAddr
						LENGTH_IS_ONE,
						&ucConfigRegVal);

	//
	// Update only masked bits in configuration register
	//
	ucConfigRegVal &= ~ ucBitMask;
	ucConfigRegVal |= (ucResolution | ucHeaterEnable);

	i2cWriteRegisters(SI7020_I2C_ADDRESS,
						WRITE_CONFIGREG_CMD, //Write Command instead of RegAddr
						LENGTH_IS_ONE,
						&ucConfigRegVal);

	return;
}

//******************************************************************************
//
//  This function does fetches temperature and RH mearurements from Sensor chip
//	and calculates their actual values in degree C and % respectively
//
//	\param[out]	pfTemp - pointer to variable that will hold value of Temperature
//	\param[out] pfRH - pointer to variable that will hold value of RH
//
//	\return none
//
//******************************************************************************
void getTempRH(float_t* pfTemp, float_t* pfRH)
{
	uint8_t ucData[4];
	uint16_t usRHCode, usTempCode;

	i2cReadRegisters( SI7020_I2C_ADDRESS,
						MEASURE_RH_CMD,
						LENGTH_IS_TWO,
						ucData);
	i2cReadRegisters( SI7020_I2C_ADDRESS,
						READ_TEMP_CMD,	// MEASURE_TEMP_CMD if not measuring RH
						LENGTH_IS_TWO,
						&ucData[2]);

	usRHCode = (ucData[0] << 8) | (ucData[1]);
	usTempCode = (ucData[2] << 8) | (ucData[3]);

	UART_PRINT("\nData Read: %x %x %x %x\n",
					ucData[0], ucData[1], ucData[2], ucData[3]);

	*pfRH = ( (125.0 * usRHCode) / 65536 ) - 6;
	*pfTemp = ( (175.72 * usTempCode) / 65536 ) - 46.85;

	return;
}

