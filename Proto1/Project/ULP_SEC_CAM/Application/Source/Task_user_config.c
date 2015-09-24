/*
 * UserConfig_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */
#include "stdlib.h"

#include "osi.h"

#include <app_fns.h>
#include "app.h"
#include "app_common.h"

#include "timer_fns.h"
#include "ota.h"
#include "watchdog.h"

extern OsiTaskHandle g_UserConfigTaskHandle;

//******************************************************************************
//	This task
//	Case (i): on power-on/soc-reset/wdt-reset
//		1. calls the user_config() if the user presses the push button
//		2. Uploads user configuration to Parse
//	Case (ii): on wake-up from hibernate
//		1. Deletes itself
//******************************************************************************
void UserConfigure_Task(void *pvParameters)
{
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
			(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET)||
			(MAP_PRCMSysResetCauseGet() == PRCM_WDT_RESET))
	{
		//Display Message to user
		RELEASE_PRINT("***SHORT PRESS: User Config mode        LONG PRESS: OTA ***\n");

		//Wait for button press. If button is not pressed at all, after 10
		//seconds (specified in main task), the Main task will delete this task
		//exiting this wait
		while(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
		{
			osi_Sleep(20);
		}
		g_ulAppStatus = USER_CONFIG_TAKING_PLACE;	//includes OTA also for now

/*		//If long press, do OTA. Else do UserConfig
		if(IsLongPress())
		{
			LED_Blink_2(.2,.2,BLINK_FOREVER);//LED_On();
			OTA_Update();
		}
		else	//short press - UserConfig
				//0 means short or no press - calling IsLongPress imdeately
				//after button press is detected, ensures it is not the no-press
				//case*/
		{
			LED_Blink_2(1,.1,BLINK_FOREVER);//LED_Blink_2(.25,.25,BLINK_FOREVER);
			RELEASE_PRINT("Config Mode\n");
			User_Configure();
			SendObject_ToParse(USER_CONFIGS);
		}

		//Main_Task polls and waits for g_ulAppStatus to become USER_CONFIG_DONE
		g_ulAppStatus = USER_CONFIG_DONE;
		while(1);
	}
	else
	{
		osi_TaskDelete(&g_UserConfigTaskHandle);
	}
}
