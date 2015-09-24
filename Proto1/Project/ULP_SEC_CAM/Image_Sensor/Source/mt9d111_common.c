/*
 * mt9d111_common.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "mt9d111.h"

//******************************************************************************
//	Does a soft reset of MT9D111
//
//	return SUCCESS or failure code
//******************************************************************************
long SoftReset_ImageSensor()
{
	long lRetVal;

	s_RegList StatusRegLst[] = {{0x00, 0x65, 0xA000},	//Bypass PLL
								{0x01, 0xC3, 0x0501},	//MCU reset
								{0x00, 0x0D, 0x0021},	//SensorCore, SOC reset
								{0x00, 0x0D, 0x0000}};	//Disable the resets

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	MT9D111Delay(24/3 + MARGIN_NUMCLKS);	//Wait 24 clocks before using I2C

	return lRetVal;
}

//******************************************************************************
//	Verifies I2C communication with MT9D111
//
//	return SUCCESS or failure code
//******************************************************************************
long Verify_ImageSensor()
{
	long lRetVal;
	uint16_t usRegVal;
	s_RegList StatusRegLst = {0x00, 0x00, 0xBADD};	// Sensor Chip Version#

	lRetVal = Register_Read(&StatusRegLst, &usRegVal);

	DEBG_PRINT("MT9D111 DeviceID: %x\n", usRegVal);

	if (usRegVal == CHIP_VERSION)
	{
		DEBG_PRINT("I2C comm. with MT9D111 SUCCESS\n");
		lRetVal = SUCCESS;
	}
	else
	{
		DEBG_PRINT("Chip Ver# error\n");
		lRetVal = MT9D111_NOT_FOUND;
	}

	return lRetVal;
}

//******************************************************************************
//	Refresh MT9D111 MCU firmware
//
//	return SUCCESS or failure code
//******************************************************************************
int32_t Refresh_mt9d111Firmware()
{
	long lRetVal;

	s_RegList StatusRegLst[] = {
									{1, 0xC6, 0xA103}, {1, 0xC8, 0x0005}	//Refresh
								};	//Disable the resets

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	return lRetVal;
}
