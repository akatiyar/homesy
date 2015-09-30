////////////////////////////////////////////////////////////////////////////////
 /*	lightSens_isl29035.c
 *	Company Name : Soliton Technologies
 *	Description  : Light Sensor chip APIs.
 *	Author		 : CS
 *	Date		 : 12/03/2015
 *	Version		 : 1.0
 *
 *
 *
 */
 ///////////////////////////////////////////////////////////////////////////////
#include <light_sens_isl29035.h>
#include "flash_files.h"

#define DEVICE_ID						0x28 //xx101xxx
#define FLASH_CONFIGS					0

//******************************************************************************
//  This function configures ISL29035
//
//	\param[in]	ucAppMode - Unique value that determines the Application mode
//	\param[in]	Lux_threshold - light on /ogg threshold
//	\param[in] Trigger - LIGHTON_TRIGGER/LIGHTOFF_TRIGGER
//	\return none
//
//	Configures registers to (i) find light intensity and (ii) give interrupt on
//		light on or off condition
//
//	\Warning: HARDWARE: Having an LED glowing close-by will simulate LightOn
////****************************************************************************
void configureISL29035(uint8_t ucAppMode,
							uint16_t Lux_threshold, uint8_t Trigger)
{
	uint8_t i = 0;
	uint8_t ucConfigArray[20];
	uint8_t ucHeader;
	uint8_t ucNoOfConfgs;

	int32_t lRetVal;

	Lux_threshold = (ADC_RANGE / LUX_RANGE) * Lux_threshold;

	ucConfigArray[i++] = CMD_1_REG;
	ucConfigArray[i++] = 0xA0;			/*( OP_MODE_ALS_ALWAYSON | INT_PERSIST)*/
	ucConfigArray[i++] = CMD_2_REG;
	ucConfigArray[i++] = (FS_RANGE_1K | ADC_RES_16);

	if(Trigger == LIGHTON_TRIGGER)
	{
		ucConfigArray[i++] = INT_LT_LSB_REG;
		ucConfigArray[i++] = 0x00;
		ucConfigArray[i++] = INT_LT_MSB_REG;
		ucConfigArray[i++] = 0x00;
		ucConfigArray[i++] = INT_HT_LSB_REG;
		ucConfigArray[i++] = Lux_threshold & 0x00FF;
		ucConfigArray[i++] = INT_HT_MSB_REG;
		ucConfigArray[i++] = (Lux_threshold & 0xFF00) >> 8;
	}
	else if(Trigger == LIGHTOFF_TRIGGER)
	{
		ucConfigArray[i++] = INT_LT_LSB_REG;
		ucConfigArray[i++] = Lux_threshold & 0x00FF;
		ucConfigArray[i++] = INT_LT_MSB_REG;
		ucConfigArray[i++] = (Lux_threshold & 0xFF00) >> 8;
		ucConfigArray[i++] = INT_HT_LSB_REG;
		ucConfigArray[i++] = 0xFF;
		ucConfigArray[i++] = INT_HT_MSB_REG;
		ucConfigArray[i++] = 0xFF;
	}

	ucNoOfConfgs = (i/2);

	if(FLASH_CONFIGS)
	{
		//Read from flash
		ReadFile_FromFlash(&ucHeader, FILENAME_SENSORCONFIGS,
							1, OFFSET_ISL29035);
		ReadFile_FromFlash(ucConfigArray, FILENAME_SENSORCONFIGS,
							ucHeader, OFFSET_ISL29035 + 1);
		ucNoOfConfgs = ucHeader/2;
	}

	for (i = 0; i < ucNoOfConfgs; i++)
	{
		lRetVal = i2cWriteRegisters(ISL29035_I2C_ADDRESS,
							ucConfigArray[i*2],
							1,
							&ucConfigArray[i*2 + 1]);
		PRINT_ON_ERROR(lRetVal);
	}
	UtilsDelay(105*80000/6);	//Wait for a single conversion time (105ms)
							//So that is the application program requests Lux
							//immediately after the config, 0 is not returned
							//by the sensor

	return;
}

//******************************************************************************
//
//  This function sets the upper and lower threshold registers in to the
//	ISL29035 deviec registers
//
//	\param[in]	upper and lower thresholds  but not in lux values
//
//	\return none
//
////*****************************************************************************
void setThreshold_lightsensor(uint16_t upper_threshold,
									uint16_t lower_threshold)
{
	uint8_t i=0;
	uint8_t low_LSB = (uint8_t) (lower_threshold & 0xff) ;
	uint8_t low_MSB = (uint8_t) ((lower_threshold>>8) & 0xff) ;
	uint8_t upper_LSB = (uint8_t) (upper_threshold & 0xff) ;
	uint8_t upper_MSB  = (uint8_t) ((upper_threshold>>8) &0xFF);
	uint8_t ucConfigArray[] = {	INT_LT_LSB_REG, low_LSB,
								INT_LT_MSB_REG, low_MSB,
								INT_HT_LSB_REG, upper_LSB,
								INT_HT_MSB_REG, upper_MSB};
	int32_t lRetVal;

	for (i = 0; i < (sizeof(ucConfigArray))/2; i++)
	{
		lRetVal = i2cWriteRegisters(ISL29035_I2C_ADDRESS,
							ucConfigArray[i*2],
							1,
							&ucConfigArray[i*2 + 1]);
		PRINT_ON_ERROR(lRetVal);
	}

	return;
}

//******************************************************************************
//  This function gets the ISL29035 light sensor data ALS
//
//	\return 16 bit light sensor data
////****************************************************************************
uint16_t getLightsensor_data(void)
{
	uint8_t databyte[2];
	uint16_t data = 0;
	uint16_t lux_reading = 0;
	int32_t lRetVal;

	//Read data regiser form ISL29035
	lRetVal = i2cReadRegisters(ISL29035_I2C_ADDRESS, DATA_LSB_REG, 2, databyte);
	PRINT_ON_ERROR(lRetVal);

	//Arrange MSB and LSB to get as one short data
	data = ((uint16_t)(databyte[1]<<8)) | databyte[0];

	//Convert read data to actual Lux value
	lux_reading = (LUX_RANGE/ADC_RANGE) * data;

	return(lux_reading);
}

//******************************************************************************
//  This function returns the interrupt gpio status of ISL29035
//
//	\return 1 if high/active; 0 if low/inactive
//
//	This function will also clear the interrupt
////****************************************************************************
uint16_t getLightsensor_intrptStatus(void)
{
	uint8_t ucStatus;
	int32_t lRetVal;

	lRetVal = i2cReadRegisters(ISL29035_I2C_ADDRESS, CMD_1_REG, 1, &ucStatus);
	PRINT_ON_ERROR(lRetVal);

	return ucStatus;
}

//******************************************************************************
//  This function verifies if the device is ISL29035
//
//	\return 0 on successful verification; ISL29035_NOT_FOUND on failure
////****************************************************************************
int32_t verifyISL29035(void)
{
	uint8_t data = 0x00;
	int32_t lRetVal;

	lRetVal = i2cReadRegisters(ISL29035_I2C_ADDRESS, DEVICE_ID_REG, 1, &data);
	PRINT_ON_ERROR(lRetVal);

	DEBG_PRINT("LightSens Device ID: %x\n", data);

	if((data & DEVICE_ID_MASK) == DEVICE_ID)
	{
		DEBG_PRINT("I2C comm with ISL29035 SUCCESS\n");
		lRetVal = SUCCESS;
	}
	else
	{
		DEBG_PRINT("ISL29035 I2C comm FAILED!\n");
		lRetVal = ISL29035_NOT_FOUND;
	}

	return lRetVal;
}

//******************************************************************************
//	This function powers down ISL29035
//
// current consumed on power down: .3uA
////****************************************************************************
int32_t PowerDown_ISL29035()
{
	int32_t lRetVal;
	uint8_t ucConfigArray[] = {CMD_1_REG, 0x00};

	lRetVal = i2cWriteRegisters(ISL29035_I2C_ADDRESS,
						ucConfigArray[0],
						LENGTH_IS_ONE,
						&ucConfigArray[1]);
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}
