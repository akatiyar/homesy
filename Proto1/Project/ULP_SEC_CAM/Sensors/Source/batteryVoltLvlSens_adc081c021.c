////////////////////////////////////////////////////////////////////////////////
 /*	batteryVoltLvlSens_adc081c021.c
 *	Company Name : Soliton Technologies
 *	Description  : ADC chip APIs for battery monitoring
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////

#include "i2c_app.h"
#include "app.h"
#include "batteryVoltLvlSens_adc081c021.h"
#include "flash_files.h"

#define FLASH_CONFIGS					0
#define HEADER_SIZE						1
//******************************************************************************
//
//	This function gets the voltage level of battery from the ADC
//
//	param[out] - pointer to a float variable to hold the battery level
//
//	return  - SUCCESS/failure code
//
//******************************************************************************
int32_t Get_BatteryVoltageLevel_ADC081C021(float_t* pfBatteryVoltage)
{
	int32_t lRetVal;
	uint8_t ucDataRegVal[2];
	uint16_t usDataRegVal;

	lRetVal = i2cReadRegisters(ADC081C021_I2C_ADDRESS, POINTER_ADDR_DATA_REG,
								LENGTH_IS_TWO, ucDataRegVal);
	usDataRegVal = (ucDataRegVal[0]<<8) + ucDataRegVal[1];
	usDataRegVal &= MASK_DATA_BITS;
	usDataRegVal >>= 4;
	*pfBatteryVoltage = usDataRegVal * ONE_LSB_EQUALS;

	return lRetVal;
}

//******************************************************************************
//
//	This function puts the ADC in Alert/Automatic conversion mode
//
//	param[out] - The lower limit of voltage which if crossed, the alert should
//	be raised
//
//	return  - SUCCESS/failure code
//
//******************************************************************************
int32_t PutInAlertMode_ADC081C021(float_t fAlertVoltageLimit)
{
	int32_t lRetVal;
	uint8_t ucVoltLowerLimit[2];
	uint16_t usVoltLowerLimit;

	uint8_t ucConfigVal;

	uint8_t ucConfigsFileContent[FILESZ_ADC081C021];
	uint8_t ucConfigsSize;

	uint8_t i = 0;

	if(!FLASH_CONFIGS)
	{
		usVoltLowerLimit = fAlertVoltageLimit / ONE_LSB_EQUALS;
		ucVoltLowerLimit[0] = (char)((usVoltLowerLimit & 0xf0) >> 4);
		ucVoltLowerLimit[1] = (char)((usVoltLowerLimit & 0x0f) << 4);
		lRetVal = i2cWriteRegisters(ADC081C021_I2C_ADDRESS,
									POINTER_ADDR_LOWERLIMIT_REG,
									LENGTH_IS_TWO, ucVoltLowerLimit);

		ucConfigVal = VALUE_CONFIGREG_CYCLETIMEFIELD_400SPS |
						VALUE_CONFIGREG_ALERTPINFIELD_ENABLE |
						VALUE_CONFIGREG_ALERTPOLARITYFIELD_HI;
		lRetVal = i2cWriteRegisters(ADC081C021_I2C_ADDRESS,
									POINTER_ADDR_CONFIG_REG,
									LENGTH_IS_ONE, &ucConfigVal);
	}
	else
	{
		//Read from Flash and write
		ReadFile_FromFlash(&ucConfigsSize, FILENAME_SENSORCONFIGS,
							HEADER_SIZE, OFFSET_ADC081C021);
		ReadFile_FromFlash(ucConfigsFileContent, FILENAME_SENSORCONFIGS,
							ucConfigsSize, OFFSET_ADC081C021 + HEADER_SIZE);
		while(i < ucConfigsSize)
		{
			lRetVal = i2cWriteRegisters(ADC081C021_I2C_ADDRESS,
									ucConfigsFileContent[i],
									ucConfigsFileContent[i+1],
									&ucConfigsFileContent[i+ucConfigsFileContent[i+1]]);
			i = i + 2 + ucConfigsFileContent[i+1];
		}
	}
	return lRetVal;
}
