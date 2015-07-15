/*
 * main_task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"

#include "osi.h"

#include "app_common.h"
#include "camera_app.h"
#include "ota.h"
#include "hibernate_related_fns.h"
#include "appFns.h"
#include "timer_fns.h"
//Try and remove these driver level includes
#include "tempRHSens_si7020.h"
#include "lightSens_isl29035.h"

#include "dbgFns.h"	//Can be removed when debug fns are not used

extern OsiTaskHandle g_UserConfigTaskHandle;

extern int32_t toggle_standby();//Tag:Remove later

void Main_Task_withHibernate(void *pvParameters)
{
	//This branch is entered only on wake up from hibernate
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		LED_On();
		UART_PRINT("\n\rI'm up\n\r");

		//This flag is necessary since parallel task also needs to access the I2C peripheral
		g_I2CPeripheral_inUse_Flag = YES;
		//Condition NOT met indicates that device hibernated while light was on
		if(!IsLightOff(LUX_THRESHOLD))
		{
			g_I2CPeripheral_inUse_Flag = NO;
			//Flag is not switched back in case where the if condition is not
			// met since the device hibernates anyways

			start_100mSecTimer();

//			Wakeup_ImageSensor();
//			Start_CameraCapture();

			CollectTxit_ImgTempRH();
		}
	}
	//This branch is entered on power on (Battery insert) or soc RST(OTA reboot)
    if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
    		(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
    	//Print the Purose of changing to this firmware here
    	UART_PRINT("*** F18 ***\n\r");
		LED_Blink(10, 1);
		LED_On();

		//Wait if User Config is happening presently
		//Will exit if UserConfig is over or UserConfig was never entered
		while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)
		{
			osi_Sleep(100);
		}
		osi_TaskDelete(&g_UserConfigTaskHandle);
		UART_PRINT("!!Application running!!\n\r");

		Check_I2CDevices();		//Remove once I2C issues are resolved

		//Configures for reading Lux and wakeup interrupt
		configureISL29035(0, LUX_THRESHOLD, NULL);
		softResetTempRHSensor();
		configureTempRHSensor();

//		//Config_And_Start_CameraCapture();
//		uint32_t ulTimeDuration_ms;
//		start_100mSecTimer();
//		Config_And_Start_CameraCapture();
//		ulTimeDuration_ms = get_timeDuration();
//		stop_100mSecTimer();
//		UART_PRINT("cam configs - %d ms\n\r", ulTimeDuration_ms);


		//start_100mSecTimer();
		Config_CameraCapture();
//		Start_CameraCapture();
//		while(1)
//		{
//			CollectTxit_ImgTempRH();
//		}
//
		Standby_ImageSensor();

//		//Tag:Remove after blank image DBG
//		start_100mSecTimer();
//		while(1)
//		{
//			Wakeup_ImageSensor();
//			Start_CameraCapture();
//			CollectTxit_ImgTempRH();
//			Standby_ImageSensor();
//		}
//		ulTimeDuration_ms = get_timeDuration();
//		stop_100mSecTimer();
//		UART_PRINT("configs over - %d ms\n\r", ulTimeDuration_ms);

		OTA_CommitImage();
	}

	HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);

	//PC never gets here
	while(1);
}


//*****************************************************************************
//
//	Main Task
//
//	Initialize and hibernate. Wake on trigger and wait for 40degree closing
//	door condition and capture image and sensor data and upload to cloud
//
//*****************************************************************************
void Main_Task(void *pvParameters)
{
	UART_PRINT("Firmware 2\n\r");
	OTA_Update_2();
	//LED_Blink(30, 1);

	LED_On();
//	while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)
//	{
//		osi_Sleep(100);	//Wait if User Config is happening presently
//	}
//	osi_TaskDelete(&g_UserConfigTaskHandle);
	UART_PRINT("!!Application running!!\n\r");

	softResetTempRHSensor();
	configureTempRHSensor();

	createAndWrite_ImageHeaderFile();
	create_JpegImageFile();

	configureISL29035(0, LUX_THRESHOLD, NULL);	//Configures for reading Lux and wakeup interrupt

	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
}
