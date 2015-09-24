/*
 * appflile_lightSensor.c
 *
 *  Created on: 18-Jun-2015
 *      Author: Chrysolin
 */

#include <light_sens_isl29035.h>
#include "app.h"

int16_t IsLightOff(uint16_t usThresholdLux)
{
	if (getLightsensor_data() <= usThresholdLux )
	{
		DEBG_PRINT("Light:off Lux:%d\n", getLightsensor_data());
		return 1;
	}
	else
		return 0;
}

int32_t PowerDown_LightSensor()
{
	int32_t lRetVal;

	DEBG_PRINT("LightSens Power down\n");
	lRetVal = PowerDown_ISL29035();
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}