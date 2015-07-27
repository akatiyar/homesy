/*
 * ImageCaptureConfigs_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "appFns.h"

#include "camera_app.h"
#include "mt9d111.h"	//Tag:Remove after debug stage

#include "osi.h"


#include "stdbool.h"
extern bool g_tempflag;

extern OsiTaskHandle g_ImageCaptureConfigs_TaskHandle;

void ImageSensor_CaptureConfigs_Task(void *pvParameters)
{
	struct u64_time time_now;
	UART_PRINT("T3\n\r");
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		//Waiting for Light sensor I2C transactions to complete
		while(g_I2CPeripheral_inUse_Flag != NO)
		{
			osi_Sleep(10);
		}
		//g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
		g_I2CPeripheral_inUse_Flag = YES;	//However, the flag is not checked hereafter

		// Wake the image sensor and begin image capture
		Wakeup_ImageSensor();

		//while(g_tempflag);
		ReStart_CameraCapture();

		//Initialize image capture
		ImagCapture_Init();

		//Tag:Timestamp Camera module up
		cc_rtc_get(&time_now);
		g_TimeStamp_CamUp = time_now.secs * 1000 + time_now.nsec / 1000000;

		//g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;

		while(g_Task3_Notification != READ_MAGNTMTRFILE_DONE)
		{
			UART_PRINT("$");
			osi_Sleep(10);
		}
		magnetometer_initialize();

		g_I2CPeripheral_inUse_Flag = NO;
		g_Task3_Notification = MAGNTMTRINIT_DONE;
	}

	// Delete the task
	osi_TaskDelete(&g_ImageCaptureConfigs_TaskHandle);
}
