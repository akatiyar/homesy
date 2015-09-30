/*
 * main_task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include <app_fns.h>
#include <hibernate.h>
#include "app.h"

#include "osi.h"

#include "app_common.h"
#include "camera_app.h"
#include "ota.h"
#include "timer_fns.h"
#include "flash_files.h"
#include "dbgFns.h"	//@Can be removed when debug fns are not used
#include "watchdog.h"
#include "jpeg.h"

#include <light_sens_isl29035.h>
#include <temp_rh_sens_si7020.h>

extern OsiTaskHandle g_UserConfigTaskHandle;

//******************************************************************************
//	This task
//	Case (i): on wake-up from hibernate
//		1. Calls the application function
//		2. Hibernates
//	Case (ii): on power-on/soc-reset/wdt-reset
//		1. Wait for 10 sec, so user can press push button and enter config mode
//		2. Do the first time configuraiton of sensors
//		3. Commit the firmware if it is running in test mode (OTA)
//		4. Hibernates
//	Aliased as task1
//******************************************************************************
void Main_Task_withHibernate(void *pvParameters)
{
	int32_t lRetVal;

	//This branch is entered only on wake up from hibernate
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		RELEASE_PRINT("\nI'm up\n");
		LED_On();

		//Enter the application functionality
		application_fn();
	}
	//This branch is entered on power on (Battery insert) or SOC reset(OTA
	//reboot/application request)
    if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
    		(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET)||
				(MAP_PRCMSysResetCauseGet() == PRCM_WDT_RESET))
	{
    	UtilsDelay(1000000);	// To ensure Firmware title is printed without
    							//discontinuity
    	RELEASE_PRINT("*** %s ***\n", FIRMWARE_VERSION);

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

		//Configure Light sensor for reading Lux. It has to be done the first
		//time
		configureISL29035(0, NULL, NULL);

		//Configure Temperature and RH sensor. Done once on power-on.
		softResetTempRHSensor();
		configureTempRHSensor();

		//Do the initial configurations for MT9D111 and then put it in standby
		Config_CameraCapture();
		Start_CameraCapture();	//Do this once. Not needed after standby wake_up
		Standby_ImageSensor();

		Modify_JPEGHeaderFile();	//In case JPEG quantisation scale is changed
									// from previous firmware

//Use the folowing code to test without hibernate - using the debugger
#ifdef USB_DEBUG
		while(1)
		{
			application_fn();
		}
#endif

  		//Commits image if running in test mode
  		lRetVal = OTA_CommitImage();
  		if (lRetVal == NEW_FIRMWARE_COMMITTED)
  		{
  			//LED_Blink_2(.2,1,BLINK_FOREVER);
  			SendObject_ToParse(FIRMWARE_VER);
  		}
	}

    // Hibernate. Setting up wake-up source is part of this function
	HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);

	//PC never gets here
	while(1);
}
