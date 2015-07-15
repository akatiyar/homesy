/*
 * ImageCaptureConfigs_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"

#include "camera_app.h"

#include "osi.h"

extern OsiTaskHandle g_ImageCaptureConfigs_TaskHandle;

void ImageSensor_CaptureConfigs_Task(void *pvParameters)
{
	UART_PRINT("T3\n\r");
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON) ||(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
		//Add some thing if necessary. Or change it to is reset cause is hibernate
	}
	else	//Wake up from hibernate case
	{
		UART_PRINT("T3 Hib\n\r");
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
		Wakeup_ImageSensor();
		Start_CameraCapture();
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;
	}

	// Delete the task in either case
	UART_PRINT("T3 Del\n\r");
	while(1)
	{
		osi_Sleep(100);
	}
	//osi_TaskDelete(&g_ImageCaptureConfigs_TaskHandle);
}
