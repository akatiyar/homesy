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
	uint32_t ulTimeDuration_ms;

	UART_PRINT("T3\n\r");
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON) ||(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
		//Add some thing if necessary. Or change it to is reset cause is hibernate
	}
	else	//Wake up from hibernate case
	{
		UART_PRINT("T3 Hib\n\r");
		//Waiting for Light sensor I2C transactions to complete
		while(g_I2CPeripheral_inUse_Flag != NO)
		{
			osi_Sleep(10);
		}
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
		g_I2CPeripheral_inUse_Flag = YES;	//However, the flag is not checked hereafter
		Wakeup_ImageSensor();
//		SoftReset_ImageSensor();
//		CameraSensorInit();	//Initial configurations
		Start_CameraCapture();
		g_I2CPeripheral_inUse_Flag = NO;
		g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;

		/*
		Config_CameraCapture();
		Standby_ImageSensor();
		while(1)
		{
			UART_PRINT("T3 Hib\n\r");
			start_100mSecTimer();

			g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
			Wakeup_ImageSensor();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("wake over - %d ms\n\r", ulTimeDuration_ms);

			start_100mSecTimer();

			Start_CameraCapture();
			g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;

			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("configs over - %d ms\n\r", ulTimeDuration_ms);

			Standby_ImageSensor();
		}
		*/
	}

	// Delete the task in either case
	UART_PRINT("T3 Del\n\r");
	while(1)
	{
		osi_Sleep(100);
	}
	//osi_TaskDelete(&g_ImageCaptureConfigs_TaskHandle);
}
