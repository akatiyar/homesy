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
#include "lightSens_isl29035.h"

//******************************************************************************
//
//  This function does configures ISL29035 registers as needed by the
//	applicaiton blueprint/mode
//
//	\param[in]	ucAppMode - Unique value that determines the Application mode
//
//	\return none
//
////****************************************************************************
void configureISL29035(uint8_t ucAppMode)
{
	uint8_t i;
	uint8_t ucConfigArray[] = { CMD_1_REG, 0xA0,
									/*( OP_MODE_ALS_ALWAYSON | INT_PERSIST)*/
								CMD_2_REG, (FS_RANGE_1K | ADC_RES_16),
								INT_LT_LSB_REG, 0x00,
								INT_LT_MSB_REG, 0x00,
								INT_HT_LSB_REG, 0XFF,
								INT_HT_MSB_REG, 0x01};

	for (i = 0; i < (sizeof(ucConfigArray))/2; i++)
	{
		i2cWriteRegisters(ISL29035_I2C_ADDRESS,
							ucConfigArray[i*2],
							1,
							&ucConfigArray[i*2 + 1]);
	}

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

	for (i = 0; i < (sizeof(ucConfigArray))/2; i++)
	{
		i2cWriteRegisters(ISL29035_I2C_ADDRESS,
							ucConfigArray[i*2],
							1,
							&ucConfigArray[i*2 + 1]);
	}

	return;
}


//******************************************************************************
//
//  This function gets the ISL29035 light sensor data ALS
//
//	\param[in]	void
//
//	\return 16 bit light sensor data
//
////****************************************************************************
uint16_t getLightsensor_data(void)
{
	uint8_t databyte[2];
	uint16_t data = 0;
	uint16_t lux_reading=0;

	i2cReadRegisters(ISL29035_I2C_ADDRESS, DATA_LSB_REG,2, databyte);
	data = ((uint16_t)(databyte[1]<<8)) | databyte[0];
	UART_PRINT("\nLux val: %x %x", databyte[1], databyte[0]);
	//LUX = [ RANGE/2^(ADC_Res) ] x data

	lux_reading = (LUX_RANGE/ADC_RANGE) * data;	//commented by uthra for testing
//	return(lux_reading);
	return data;
}


//******************************************************************************
//
//  This function returns the interrupt gpio status of ISL29035
//
//	\param[in]	void
//
//	\return 16 bit 1 is high/active 0 is low/inactive
//
////****************************************************************************
uint16_t getLightsensor_intrptStatus(void)
{
	uint8_t ucStatus;

	i2cReadRegisters(ISL29035_I2C_ADDRESS, CMD_1_REG, 1, &ucStatus);

	return ucStatus;
}


//******************************************************************************
//
//  This function verifies if the device is ISL29035
//
//	\param[in]	void
//
//	\return 16 bit light sensor data
//
////****************************************************************************
uint16_t verifyISL29035(void)
{
	uint8_t data = 0x00;

	i2cReadRegisters(ISL29035_I2C_ADDRESS, DEVICE_ID_REG, 1, &data);
	UART_PRINT("LightSensor Device ID: %x\n\r", data);

	return( !((data & DEVICE_ID_MASK) == DEVICE_ID) );
}

