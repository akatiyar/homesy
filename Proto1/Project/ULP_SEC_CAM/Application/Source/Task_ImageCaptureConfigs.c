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
#include "timer_fns.h"

#include "stdbool.h"

#include "flash_files.h"
extern bool g_tempflag;

extern OsiTaskHandle g_ImageCaptureConfigs_TaskHandle;

void ImageSensor_CaptureConfigs_Task(void *pvParameters)
{
	struct u64_time time_now;
	uint32_t ulTimeDuration_ms;

	uint16_t* ImageConfigData = (uint16_t *) g_image_buffer;
	ImageConfigData = ImageConfigData + (OFFSET_MT9D111/sizeof(uint16_t));

	//DEBG_PRINT("T3\n\r");
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		//
		//	Initialize Image sensor
		//
		//Waiting for Light sensor I2C transactions to complete
		while(g_I2CPeripheral_inUse_Flag != NO)
		{
			osi_Sleep(10);
		}
		//g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
		g_I2CPeripheral_inUse_Flag = YES;	//However, the flag is not checked hereafter

		Wakeup_ImageSensor();		//Wake the image sensor

		//
		//	Initialize Angle Check
		//
		//Ensure/Wait till magnetometer calibration are read from flash and put
		//in global variables
		while(g_Task3_Notification != READ_MAGNTMTRFILE_DONE)
		{
			DEBG_PRINT("$");
			osi_Sleep(5);
		}
		//Configure FXOS and read out first few angle values
		g_Task1_Notification = MAGNETOMETERINIT_STARTED;
		magnetometer_initialize();

		while(g_Task3_Notification != READ_SENSORCONFIGFILE_DONE)
		{
			DEBG_PRINT("%");
			osi_Sleep(10);
		}

		ReStart_CameraCapture(ImageConfigData);	//Restart image capture

		g_I2CPeripheral_inUse_Flag = NO;

		//Initialize image capture
		memset((void*)g_image_buffer, 0x00, IMAGE_BUF_SIZE_BYTES);
		ImagCapture_Init();
		//Tag:Timestamp Camera module up
		cc_rtc_get(&time_now);
		g_TimeStamp_CamUp = time_now.secs * 1000 + time_now.nsec / 1000000;
		//g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;

		ulTimeDuration_ms = get_timeDuration();
		stop_100mSecTimer();
		DEBG_PRINT("Till angle: %dms\n", ulTimeDuration_ms);
		//ulTimeDuration_ms = 0;
//		start_100mSecTimer();

		//
		//	Get angle and check door position
		//
		//This part runs only until the main task finishes opening image file,
		//so that in case the door open event happens during file open operation,
		//it will be detected
		while(g_Task1_Notification != IMAGEFILE_OPEN_COMPLETE)
		{
			g_I2CPeripheral_inUse_Flag = YES;
			get_angle();
			check_doorpos();
			if(g_flag_door_closing_45degree)
			{
				g_ucReasonForFailure = DOOR_SHUT_DURING_FILEOPEN;
			}
			g_I2CPeripheral_inUse_Flag = NO;
		}

//		ulTimeDuration_ms = get_timeDuration();
//		stop_100mSecTimer();
//		DEBG_PRINT("Fileopen over: %dms\n", ulTimeDuration_ms);
		//g_I2CPeripheral_inUse_Flag = NO;
	}

	// Delete the task
	osi_TaskDelete(&g_ImageCaptureConfigs_TaskHandle);
}
