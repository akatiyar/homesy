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

#define ADC081C021_I2C_ADDRESS 					0x54	//SOT-6

#define POINTER_ADDR_DATA_REG					0X00
#define POINTER_ADDR_ALERTSTATUS_REG			0X01
#define POINTER_ADDR_CONFIG_REG					0X02
#define POINTER_ADDR_LOWERLIMIT_REG				0X03
#define POINTER_ADDR_HIGHERLIMIT_REG			0X04
#define POINTER_ADDR_HISTERISIS_REG				0X05
#define POINTER_ADDR_LOWESTDATA_REG				0X06
#define POINTER_ADDR_HIGHESTDATA_REG			0X07

#define RESISTOR_R1								(100.0F)//in KOhms. Tag:Schematic
#define RESISTOR_R2								(47.0F)	//in KOhms. Tag:Schematic

#define VREF									(3.0F)	//Supply Vcc to chip = Reference
#define ONE_LSB_EQUALS							(VREF/256.0F)	// 2^8 = 256

#define MASK_DATA_BITS							0x0ff0

#define VALUE_CONFIGREG_CYCLETIMEFIELD_400SPS	0xE0

#define	VALUE_CONFIGREG_ALERTPINFIELD_ENABLE	0X04

#define	VALUE_CONFIGREG_ALERTPOLARITYFIELD_LO	0X00
#define	VALUE_CONFIGREG_ALERTPOLARITYFIELD_HI	0X00
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
	uint8_t ucADCValue;
//	uint16_t usDataRegVal;

	lRetVal = i2cReadRegisters(ADC081C021_I2C_ADDRESS, POINTER_ADDR_DATA_REG,
								LENGTH_IS_TWO, ucDataRegVal);
//	usDataRegVal = (ucDataRegVal[0]<<8) + ucDataRegVal[1];
//	usDataRegVal &= MASK_DATA_BITS;
//	usDataRegVal >>= 4;
	ucADCValue = (ucDataRegVal[0]<<4) | (ucDataRegVal[1]>>4);
	*pfBatteryVoltage = ((float_t)ucADCValue * ONE_LSB_EQUALS) + 0.5 * ONE_LSB_EQUALS;	//Analog Voltage(VA)
	(*pfBatteryVoltage) = (*pfBatteryVoltage) * (RESISTOR_R1+RESISTOR_R2)/RESISTOR_R2;	//VA = VBAT*R2/(R1+R2)

	//Assuming a linear relation between Voltage and Charge, which is usually not the case


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
