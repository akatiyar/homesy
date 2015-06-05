
#include "common.h"
#include "app.h"
#include "prcm.h"

#include "lightSens_isl29035.h"
#include "accelomtrMagntomtr_fxos8700.h"

#include "hibernate_related_fns.h"
#include "app_common.h"
#include "stdbool.h"

#define SEC_PER_MINUTE					60
#define RTCCLKS_PER_SEC					32768


//******************************************************************************
//	This function puts the device in hibernate
//
//	param[in]	ucWakeSources - choice of source for wake up from hibernate. Can
//						be one of the following macros or the or of the two
//							ENABLE_TIMER_WAKESOURCE
//							ENABLE_GPIO02_WAKESOURCE
//	param[in]	ucGPIOInterruptType-Type of GPIO Signal taken as wakeup signal.
//						Can take one of the following Macros as its value
//							PRCM_HIB_LOW_LEVEL
//							PRCM_HIB_HIGH_LEVEL
//							PRCM_HIB_FALL_EDGE
//							PRCM_HIB_RISE_EDGE
//	param[in]	fHibIntervalInMinutes-Hibernation interval before the Slow Clock
//						counter wakes the device up fom Hibernate
//
//	Description:
//		1. Disable the wakeup sources
//		2. Enable the wakeup source(s)
//		3. Sets Hibernate interval incase time based interrupt is enabled
//		4. Sets up type of wake trigger, if GPIO WakeSource is enabled
//		5. Prepares peripheral devices to give interrupt
//		6. Goes into hibernate
//
//	NOTE: The device
//******************************************************************************
void HIBernate(uint32_t ucWakeSources,
				uint32_t ucGPIOInterruptType,
				float_t fHibIntervalInMinutes)
{

    //
    // Setup Wake Source
    //
	MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_GPIO2|PRCM_HIB_SLOW_CLK_CTR);
	MAP_PRCMHibernateWakeupSourceEnable(ucWakeSources);

	if(ucWakeSources&PRCM_HIB_SLOW_CLK_CTR)
	{
		MAP_PRCMHibernateIntervalSet(fHibIntervalInMinutes*((float_t)SEC_PER_MINUTE)*((float_t)RTCCLKS_PER_SEC));
	}
	if(ucWakeSources&PRCM_HIB_GPIO2)
	{
		MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, ucGPIOInterruptType);

		bool temp = true;
		uint8_t count=0;
		// (0x01FF) is trial val
		while(temp)
		{
			if (getLightsensor_data() < (0x01FF))
			{
				count++;
				if(count>10)
					temp = false;

				UtilsDelay((0.5)*80000000/6);
			}
			else
				count=0;
		}
		//
		//	Configure or clear interrupts of wake-up trigger sensors
		//
		UART_PRINT("Fridge Light Off");
		LED_Blink(5, 0.5);
		sensorsTriggerSetup();
		UART_PRINT("Cleared sensors\n\r");
	}

	DBG_PRINT("HIB: Entering HIBernate...\n\r"
        		"***OPEN DOOR TO CAPTURE IMAGE***\n\r");
    MAP_UtilsDelay(80000);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}
/*{
    //
    // Setup Wake Source
    //
	if(ucEnableSlowClkCntrWake)
	{
		//MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2|PRCM_HIB_SLOW_CLK_CTR);
		MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
		MAP_PRCMHibernateIntervalSet(fHibIntervalInMinutes*SEC_PER_MINUTE*RTCCLKS_PER_SEC);
	}
	else
	{
		MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_SLOW_CLK_CTR);
				//Otherwise old setting of RTC timer Wake trigger will be
				//retained
		//MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
	}

	if(ucEnableGPIOWake)
	{
		MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
		MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, ucGPIOInterruptType);

		//
		//	Configure or clear interrupts of wake-up trigger sensors
		//
		sensorsTriggerSetup();
		UART_PRINT("Cleared sensors\n\r");
	}
	else
	{
		MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_GPIO2);
	}

    DBG_PRINT("HIB: Entering HIBernate...\n\r"
        		"***OPEN DOOR TO CAPTURE IMAGE***\n\r");
    MAP_UtilsDelay(80000);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}*/


void sensorsTriggerSetup()
{
	//
	// If Light sensor is to be used
	//
	UART_PRINT("\n\rLIGHT SENSOR:\n\r");
	verifyISL29035();
	configureISL29035(0);
	getLightsensor_intrptStatus(); //To clear the interrupt
	UART_PRINT("Configured Light Sensor for wake up\n\r");

	//
	// If Accelerometer trigger is to be used
	//
	//	verifyAccelMagnSensor();
	//	clearAccelMotionIntrpt();
	//	configureFXOS8700(MODE_ACCEL_INTERRUPT);
	//	UART_PRINT("Configured Accelerometer for wake up\n\r");
}
