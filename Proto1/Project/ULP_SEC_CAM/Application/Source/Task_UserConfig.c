/*
 * UserConfig_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"

#include "app_common.h"
#include "timer_fns.h"
#include "ota.h"
#include "appFns.h"

#include "watchdog.h"

// TI library headers that are not included in app.h
#include "osi.h"

#define TIMEOUT_LONGPRESS		(30)	//3 sec press is defined as long press.

extern OsiTaskHandle g_UserConfigTaskHandle;
extern int32_t sendUserConfigData();

void UserConfigure_Task(void *pvParameters)
{
	//LED_On();
	UtilsDelay(1000000);	// To ensure Firmware title is printed first
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)||
			(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET)||
			(MAP_PRCMSysResetCauseGet() == PRCM_WDT_RESET))
	{
		//Display Message to user
		RELEASE_PRINT("***SHORT PRESS: User Config mode        LONG PRESS: OTA ***\n");

		//Indicate to the user that he can press the button now
		LED_Blink_2(0.5, 0.5, BLINK_FOREVER);

		//Wait for button press. If button is not pressed at all, after 10
		//seconds (specified in main task), the Main task will delete this task
		//exiting this wait
		while(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
		{
			osi_Sleep(20);
		}
		g_ulAppStatus = USER_CONFIG_TAKING_PLACE;	//includes OTA also for now

		//If long press, do OTA. Else do UserConfig
		if(IsLongPress())
		{
			LED_On();
			OTA_Update();
		}
		else	//short press - UserConfig
				//0 means short or no press - calling IsLongPress imdeately
				//after button press is detected, ensures it is not the no-press
				//case
		{
			LED_Blink_2(.25,.25,BLINK_FOREVER);
			User_Configure();
			sendUserConfigData();
		}

		//Main_Task polls and waits for g_ulAppStatus to become USER_CONFIG_DONE
		g_ulAppStatus = USER_CONFIG_DONE;
		while(1);
	}
	else
	{
		osi_TaskDelete(&g_UserConfigTaskHandle);
	}
	//osi_TaskDelete(&g_UserConfigTaskHandle);
}

//Returns 1 if long-press and 0 if short-press or no-press
int16_t IsLongPress()
{
	// Logic for detecting press type: long(>= 3sec) or short (< 3sec)
	start_100mSecTimer();
	while(1)
	{
		if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
		{
			break;
		}

		// This code is for avoiding false button release detection due to
		//	debounce. If pin is found to be high, it is checked again after
		//	10 sec. It is repeatedly checked 3 times. If it is high 3
		//	consecutive times, then it is not a false release of button.
		uint8_t i;
		for(i = 0; i < 3; i++)
		{
			if(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
			{
				//UtilsDelay(12*80000/6);	//12 millisec delay
				osi_Sleep(100);	//suspend the task for 10milli sec
			}
			else
			{
				//DEBG_PRINT("pinlow\n\r");
				break;
			}
		}
		if (i>=3)
		{
			break;
		}

		//UtilsDelay(12*80000/6);	//12 millisec delay
	}
	stop_100mSecTimer();

	if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
	{
		DEBG_PRINT("Long press\n", Elapsed_100MilliSecs);
		return 1;
	}
	else //if (Elapsed_100MilliSecs < TIMEOUT_LONGPRESS)
	{
		DEBG_PRINT("Short press\n", Elapsed_100MilliSecs);
		return 0;
	}

	//return 0;	//Control doesnot get here
}
