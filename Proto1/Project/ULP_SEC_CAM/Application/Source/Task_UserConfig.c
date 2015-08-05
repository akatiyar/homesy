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
	if ((MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON) ||(MAP_PRCMSysResetCauseGet() == PRCM_SOC_RESET))
	{
		UART_PRINT("***SHORT PRESS-CONFIGURE THRU PHONE APP***\n\r"
						"***LONG PRESS-OTA***\n\r");
		while(GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))
		{
			osi_Sleep(20);
		}
		g_ulAppStatus = USER_CONFIG_TAKING_PLACE;	//includes OTA also for now

		// Logic for detecting press type: long(>= 3sec) or short (< 3sec)
		start_100mSecTimer();
		while(1)
		{
			if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
			{
				UART_PRINT("Time out\n\r");
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
					UART_PRINT("pinhigh DB %d\n\r", i);
					//UtilsDelay(12*80000/6);	//12 millisec delay
					osi_Sleep(100);	//suspend the task for 10milli sec
				}
				else
				{
					//UART_PRINT("pinlow\n\r");
					break;
				}
			}
			if (i>=3)
			{
				UART_PRINT("pin high... short press\n\r");
				break;
			}

			//UtilsDelay(12*80000/6);	//12 millisec delay
		}
		stop_100mSecTimer();

		if(Elapsed_100MilliSecs >= TIMEOUT_LONGPRESS)
		{
			UART_PRINT("Long press : %d\n\r", Elapsed_100MilliSecs);
			LED_On();
			OTA_Update();
		}
		else if (Elapsed_100MilliSecs < TIMEOUT_LONGPRESS)
		{
			UART_PRINT("Short press : %d\n\r", Elapsed_100MilliSecs);
			LED_Blink_2(.25,.25,BLINK_FOREVER);
			g_ulAppTimeout_ms = USERCONFIG_TIMEOUT;
			User_Configure();
			sendUserConfigData();
			UART_PRINT("User Cofig Mode EXIT\n\r");
		}

		//Main_Task polls and waits for g_ulAppStatus to become USER_CONFIG_DONE
		g_ulAppStatus = USER_CONFIG_DONE;
		//while(1);
	}
	else
	{
		//osi_TaskDelete(&g_UserConfigTaskHandle);
	}
	osi_TaskDelete(&g_UserConfigTaskHandle);
}

