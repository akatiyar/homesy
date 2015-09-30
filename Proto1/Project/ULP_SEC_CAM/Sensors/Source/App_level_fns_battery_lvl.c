/*
 * appFns_BatteryLevelSense.c
 *
 *  Created on: 30-Jun-2015
 *      Author: Chrysolin
 */

#include <battery_volt_lvl_adc081c021.h>
#include "app.h"

#define RESISTOR_R1								(100.0F)//in KOhms. Tag:Schematic
#define RESISTOR_R2								(47.0F)	//in KOhms. Tag:Schematic

#define VREF									(3.0F)	//Supply Vcc to chip = Reference voltage
#define ONE_LSB_EQUALS							(VREF/ADC_RESOLUTION)

#define BATTERY_MAXIMUM_CAPACITY				1400	//in mAh	//Tag. Should this be made equal to 1240?
#define V2C_LUT_SIZE							11

typedef struct VoltageToChargeLookUp
{
	float_t fVbat;
	uint8_t ucRemainingChargePercent;	//in mAh
}V2CLookUp;

//******************************************************************************
//
//	This function returns the percent of Battery left
//
//	params [none]
//	return Percentatge of Battery left
//
//	Steps:
//	1. Get the ADC value of VA from ADC081C021. VA is voltage at input pin of
//		ADC
//	2. Calculate VA
//	3. Then find VBAT from VA. VBAT is voltage across battery
//	4. Look up the battery percent left that corresponds to VBAT from the LUT
//
//	The function does NOT need any configuration to be done before being called
//
//******************************************************************************
uint8_t Get_BatteryPercent()
{
	float_t fBatteryVoltage;
	uint8_t ChargePercent;
	uint8_t ucADCValue;
	uint8_t i;
	static const V2CLookUp V2CEntry[] = {
											{2.9,	100},
											{2.625,	99},
											{2.61,	96},
											{2.6,	61},
											{2.56,	46},
											{2.47,	32},
											{2.375,	24},
											{2.37,	19},
											{2.23,	16},
											{2.15,	14},
											{2,		11},
											{1.7,	5},
											{0,		0}
										};

	//	Get the ADC value of VA from ADC081C021
	Get_BatteryVoltageLevel_ADC081C021(&ucADCValue);

	//	Calculate VA
	fBatteryVoltage = ((float_t)ucADCValue * ONE_LSB_EQUALS) + 0.5 * ONE_LSB_EQUALS;	//Analog Voltage(VA)

	//	Find VBAT from VA. VA = VBAT*R2/(R1+R2)
	(fBatteryVoltage) = (fBatteryVoltage) * (RESISTOR_R1 + RESISTOR_R2) / RESISTOR_R2;

	//	Look up LUT for battery percent corresponding to VA
	i = 0;
	while(i<sizeof(V2CEntry)/sizeof(V2CLookUp))
	{
		if( fBatteryVoltage > V2CEntry[i].fVbat )
		{
			ChargePercent = V2CEntry[i].ucRemainingChargePercent;
			break;
		}
		i++;
	}

	DEBG_PRINT("ADC Value= %xH, VBAT = %3.3f, PercentCharge = %d\n", ucADCValue, fBatteryVoltage, ChargePercent);

	return ChargePercent;
}

//******************************************************************************
//
//	This function returns the voltage across battery
//
//	params [none]
//	return battery voltage
//
//	Steps:
//	1. Get the ADC value of VA from ADC081C021. VA is voltage at input pin of
//		ADC
//	2. Calculate VA
//	3. Then find VBAT from VA. VBAT is voltage across battery
//
//	The function does NOT need any configuration to be done before being called
//
//******************************************************************************
float_t Get_BatteryVoltage()
{
	float_t fBatteryVoltage;
	uint8_t ucADCValue;

	//	Get the ADC value of VA from ADC081C021
	Get_BatteryVoltageLevel_ADC081C021(&ucADCValue);

	//	Calculate VA
	fBatteryVoltage = ((float_t)ucADCValue * ONE_LSB_EQUALS) + 0.5 * ONE_LSB_EQUALS;	//Analog Voltage(VA)

	//	Find VBAT from VA. VA = VBAT*R2/(R1+R2)
	(fBatteryVoltage) = (fBatteryVoltage) * (RESISTOR_R1 + RESISTOR_R2) / RESISTOR_R2;	//VA = VBAT*R2/(R1+R2)

	DEBG_PRINT("ADC Value= %xH, VBAT = %3.3f\n", ucADCValue, fBatteryVoltage);

	return fBatteryVoltage;
}

