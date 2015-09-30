/*
 * appFns_IOExpander.c
 *
 *  Created on: 30-Jul-2015
 *      Author: Chrysolin
 */

#include <io_expander_pcf8574.h>
#include "app.h"

//******************************************************************************
//	Clears interrupt or resets and reactivates the interrupt circuit
//******************************************************************************
int32_t ClearInterrupt_IOExpander()
{
	uint8_t ucPortData;

	ReadPorts_pcf8574(&ucPortData);

	return 0;
}

//******************************************************************************
//	Check if the interrupt is from Battery ADC or not
//******************************************************************************
int8_t IsInterruptFromBatteryADC()
{
	uint8_t ucPortData;

	ReadPorts_pcf8574(&ucPortData);

	if(ucPortData & BATTERY_ADC_TRIGGER)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//******************************************************************************
//	Check if the interrupt is from LightSensor or not
//******************************************************************************
int8_t IsInterruptFromLightSensor()
{
	uint8_t ucPortData;

	ReadPorts_pcf8574(&ucPortData);

	if(ucPortData & LIGHTSENSOR_TRIGGER)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
