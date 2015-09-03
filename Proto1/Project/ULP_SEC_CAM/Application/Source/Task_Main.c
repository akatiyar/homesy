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
//Tag:Try and remove these driver level includes
#include "tempRHSens_si7020.h"
#include "lightSens_isl29035.h"

#include "dbgFns.h"	//Can be removed when debug fns are not used

#include "watchdog.h"

#include "jpeg.h"

extern OsiTaskHandle g_UserConfigTaskHandle;

extern int32_t sendUserConfigData();

void Main_Task_withHibernate(void *pvParameters)
{
	int32_t lRetVal;

	WDT_init();

	//This branch is entered only on wake up from hibernate
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		RELEASE_PRINT("\nI'm up\n");

		//Enter the application funcitonality
		application_fn();
	}
	//This branch is entered on power on (Battery insert) or SOC reset(OTA reboot)
    if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
    		(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET)||
				(MAP_PRCMSysResetCauseGet() == PRCM_WDT_RESET))
	{
    	//Give firmware ID/Version or gist of firmware change here
    	RELEASE_PRINT("*** %s ***\n\r", FIRMWARE_VERSION);

    	//Give time to press the push button for OTA or MobileApp config
#ifndef	DEBUG_MODE
    	osi_Sleep(10000);	//10 second wait for user to press a button
#endif

		//Wait if User Config is happening presently
		//Loop will exit if UserConfig is over or UserConfig was never entered
		while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)
		{
			osi_Sleep(100);
		}
		osi_TaskDelete(&g_UserConfigTaskHandle);

    	LED_On();
		RELEASE_PRINT("App Inits\n");

		Check_I2CDevices();		//Tag:Remove once I2C issues are resolved

		//Configure Light sensor for reading Lux. It has to be done the first time
		configureISL29035(0, NULL, NULL);

		//Configure Temperature and RH sensor. Done once on power-on.
		softResetTempRHSensor();
		configureTempRHSensor();

		//Do the initial configurations for MT9D111 and then put it in standby
		Config_CameraCapture();
		Start_CameraCapture();	//Do this once. Not needed after standby wake_up
		Standby_ImageSensor();

		Modify_JPEGHeaderFile();

//Use the folowing code to test without hibernate
#ifdef USB_DEBUG
		while(1)
		{
			application_fn();
			MAP_UtilsDelay(1000);
		}
#endif

  		//Commits image if running in test mode
  		lRetVal = OTA_CommitImage();
  		if (lRetVal == NEW_FIRMWARE_COMMITTED)
  		{
  			SendObject_ToParse(FIRMWARE_VER);
  		}
	}

    /*//A way to reset the device without removing the battery
    if(!GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
    {
    	RELEASE_PRINT("Reset button press detected\nRelease button now\n");
    	LED_Off();
    	while(!GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
    	{
    		osi_Sleep(100);
    	}
    	RELEASE_PRINT("Button released\n");
#ifdef WATCHDOG_ENABLE
    	Reset_byStarvingWDT();
#else
    	PRCMSOCReset();
#endif
    }*/

    // Hibernate. Setting up wake-up source is part of this function
	HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);

	//PC never gets here
	while(1);
}
