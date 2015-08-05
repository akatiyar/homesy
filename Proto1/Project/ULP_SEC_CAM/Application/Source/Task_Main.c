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

extern OsiTaskHandle g_UserConfigTaskHandle;

void Main_Task_withHibernate(void *pvParameters)
{
	WDT_init();

	//This branch is entered only on wake up from hibernate
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		LED_On();
		UART_PRINT("\n\rI'm up\n\r");

		//Enter the application funcitonality
		application_fn();
	}
	//This branch is entered on power on (Battery insert) or soc RST(OTA reboot)
    if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
    		(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET)||
				(MAP_PRCMSysResetCauseGet() == PRCM_WDT_RESET))
	{
    	//Give firmware ID/Version or gist of firmware change here
    	UART_PRINT("*** %s ***\n\r", FIRMWARE_VERSION);

    	//Give time to press the push button for OTA or MobileApp config
		//LED_Blink(10, 1);
    	LED_Blink_2(0.5, 0.5, BLINK_FOREVER);
    	osi_Sleep(10000);	//10 second wait for user to press a button

		//Wait if User Config is happening presently
		//Loop will exit if UserConfig is over or UserConfig was never entered
		while(g_ulAppStatus == USER_CONFIG_TAKING_PLACE)
		{
			osi_Sleep(100);
		}
		//osi_TaskDelete(&g_UserConfigTaskHandle);
		UART_PRINT("!!Application running!!\n\r");

		Check_I2CDevices();		//Tag:Remove once I2C issues are resolved

		//DBG - having to read out first few angle values
/*		NWP_SwitchOn();
		angleCheck_Initializations();
		RdSensData_Init();
		//uint32_t ulTimeDuration_ms;
		while(1)
		{
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();

			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
			get_angle();
			check_doorpos();
		}
		while(1)
		{
			start_100mSecTimer();	//Tag:Remove when waketime optimization is over
			get_angle();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("1. Time - %d ms\n\r", ulTimeDuration_ms);

			start_100mSecTimer();	//Tag:Remove when waketime optimization is over
			get_angle();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("2. Time - %d ms\n\r", ulTimeDuration_ms);

			start_100mSecTimer();	//Tag:Remove when waketime optimization is over
			get_angle();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("3. Time - %d ms\n\r", ulTimeDuration_ms);

			start_100mSecTimer();	//Tag:Remove when waketime optimization is over
			get_angle();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			UART_PRINT("4. Time - %d ms\n\r", ulTimeDuration_ms);
		}*/

		//Configure Light sensor for reading Lux. It has to be done the first time
		configureISL29035(0, LUX_THRESHOLD, NULL);

		//Configure Temperature and RH sensor. Done once on power-on.
		softResetTempRHSensor();
		configureTempRHSensor();

		//Do the initial configurations for MT9D111 and then put it in standby
		Config_CameraCapture();
		Start_CameraCapture();	//Do this once. Not needed after standby wake_up
		Standby_ImageSensor();

//Use the folowing code to test without hibernate
/*
  		while(1)
		{
  			application_fn();
  			//MAP_UtilsDelay(3*80000000/6);
		}
*/

  		//Commits image if running in test mode
		OTA_CommitImage();
	}

    // Hibernate. Setting up wake-up source is part of this function
	HIBernate(ENABLE_GPIO_WAKESOURCE, FALL_EDGE, NULL, NULL);

	//PC never gets here
	while(1);
}
