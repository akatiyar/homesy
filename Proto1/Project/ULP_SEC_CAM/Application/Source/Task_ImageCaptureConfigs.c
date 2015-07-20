/*
 * ImageCaptureConfigs_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"

#include "camera_app.h"
#include "mt9d111.h"	//Tag:Remove after debug stage

#include "osi.h"

extern OsiTaskHandle g_ImageCaptureConfigs_TaskHandle;

void ImageSensor_CaptureConfigs_Task(void *pvParameters)
{
	UART_PRINT("T3\n\r");
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		//Waiting for Light sensor I2C transactions to complete
		while(g_I2CPeripheral_inUse_Flag != NO)
		{
			osi_Sleep(10);
		}
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
		g_I2CPeripheral_inUse_Flag = YES;	//However, the flag is not checked hereafter

		// Wake the image sensor and begin image capture
		Wakeup_ImageSensor();
		Start_CameraCapture();

		g_I2CPeripheral_inUse_Flag = NO;
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;
	}

	// Delete the task
	osi_TaskDelete(&g_ImageCaptureConfigs_TaskHandle);
}
