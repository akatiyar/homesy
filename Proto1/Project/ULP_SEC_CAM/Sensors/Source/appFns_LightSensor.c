/*
 * appflile_lightSensor.c
 *
 *  Created on: 18-Jun-2015
 *      Author: Chrysolin
 */


#include "app.h"
#include "lightSens_isl29035.h"

int16_t IsLightOff(uint16_t usThresholdLux)
{
	if (getLightsensor_data() <= usThresholdLux )
	{
		UART_PRINT("Light's off. %d Lux\n\r", getLightsensor_data());
		return 1;
	}
	else
		return 0;
}
