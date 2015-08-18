/*
 * appFns_BatteryLevelSense.c
 *
 *  Created on: 30-Jun-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "batteryVoltLvlSens_adc081c021.h"

#define RESISTOR_R1								(100.0F)//in KOhms. Tag:Schematic
#define RESISTOR_R2								(47.0F)	//in KOhms. Tag:Schematic

#define VREF									(3.0F)	//Supply Vcc to chip = Reference voltage
#define ONE_LSB_EQUALS							(VREF/ADC_RESOLUTION)	// 2^8 = 256

#define BATTERY_MAXIMUM_CAPACITY				1400	//in mAh	//Tag. Should this be made equal to 1240?
#define V2C_LUT_SIZE							11

typedef struct VoltageToChargeLookUp
{
	float_t fVbat;
	uint8_t ucRemainingChargePercent;	//in mAh
}V2CLookUp;


//******************************************************************************
//	params [none]
//	return Percentatge of Battery left
//
//	This function returns the percent of Battery left
//	1. Get the ADC value of VA from ADC081C021
//	2. Calculate VA
//	3. Then VBAT from VA
//	4. Find the discharge value that corresponds to VBAT using the LUT
//	5. Calculate percentage charge left from discharge value
//
//	The function does NOT need any cnfiguration to be done before being called
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

	Get_BatteryVoltageLevel_ADC081C021(&ucADCValue);

	fBatteryVoltage = ((float_t)ucADCValue * ONE_LSB_EQUALS) + 0.5 * ONE_LSB_EQUALS;	//Analog Voltage(VA)
	(fBatteryVoltage) = (fBatteryVoltage) * (RESISTOR_R1 + RESISTOR_R2) / RESISTOR_R2;	//VA = VBAT*R2/(R1+R2)

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

