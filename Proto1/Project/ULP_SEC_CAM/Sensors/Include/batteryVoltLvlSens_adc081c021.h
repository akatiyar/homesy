////////////////////////////////////////////////////////////////////////////////
 /*	batteryVoltLvlSens_adc081c021.h
 *	Company Name : Soliton Technologies
 *	Description  : ADC chip APIs for battery monitoring
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////


#ifndef BATTERYVOLTLVLSENS_ADC081C021_H_
#define BATTERYVOLTLVLSENS_ADC081C021_H_

#define ADC081C021_I2C_ADDRESS 					0x54	//SOT-6

#define POINTER_ADDR_DATA_REG					0X00
#define POINTER_ADDR_ALERTSTATUS_REG			0X01
#define POINTER_ADDR_CONFIG_REG					0X02
#define POINTER_ADDR_LOWERLIMIT_REG				0X03
#define POINTER_ADDR_HIGHERLIMIT_REG			0X04
#define POINTER_ADDR_HISTERISIS_REG				0X05
#define POINTER_ADDR_LOWESTDATA_REG				0X06
#define POINTER_ADDR_HIGHESTDATA_REG			0X07

#define VA_VOLTAGE								(3.0)	//Supply Vcc to chip
#define ONE_LSB_EQUALS							(VA_VOLTAGE/256.0)	// 2^8 = 256

#define MASK_DATA_BITS							0x0ff0

#define VALUE_CONFIGREG_CYCLETIMEFIELD_400SPS	0xE0

#define	VALUE_CONFIGREG_ALERTPINFIELD_ENABLE	0X04

#define	VALUE_CONFIGREG_ALERTPOLARITYFIELD_LO	0X00
#define	VALUE_CONFIGREG_ALERTPOLARITYFIELD_HI	0X00

#include "app.h"
#include "math.h"



int32_t PutInAlertMode_ADC081C021(float_t fAlertVoltageLimit);

int32_t Get_BatteryVoltageLevel_ADC081C021(float_t* pfBatteryVoltage);





#endif /* BATTERYVOLTLVLSENS_ADC081C021_H_ */
