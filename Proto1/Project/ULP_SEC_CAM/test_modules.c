/*
 * test_modules.c
 *
 *  Created on: 19-May-2015
 *      Author: Chrysolin
 */
#include "app.h"

#include "mt9d111.h"
#include "camera_app.h"
#include "network_related_fns.h"
#include "parse.h"
#include "parse_uploads.h"
#include "hibernate_related_fns.h"
#include "flash_files.h"
#include "app_common.h"
#include "timer_fns.h"
#include "wifi_provisioing_thruAPmode.h"

#include "prcm.h"

#include "test_modules.h"

void Test_Provisioning_thruAP()
{
	int32_t lRetVal;

	ProvisioningAP();

	//Testing WiFi Configs in Flash - DBG
	ConfigureSimpleLinkToDefaultState();
	UtilsDelay(8000000);
	//	Start NWP for flash read
	lRetVal = sl_Start(0, 0, 0);
	if (lRetVal < 0)
	{
	   UART_PRINT("Failed to start the device \n\r");
	   LOOP_FOREVER();
	}
	ConnectToNetwork_STA();

	while(1);
}

void TestOrProfile_CameraModuleStandby()
{
	int32_t lRetVal;
	UART_PRINT("\n\rCAMERA MODULE:\n\r");
	CamControllerInit();	// Init parallel camera interface of cc3200
							// since image sensor needs XCLK for
							//its I2C module to work

	UtilsDelay(24/3 + 10);	// Initially, 24 clock cycles needed by MT9D111
							// 10 is margin

	SoftReset_ImageSensor();

	UART_PRINT("Cam Sensor Init \n\r");
	CameraSensorInit();

	UART_PRINT("Standby \n\r");
	//EnterStandby_mt9d111();
	Standby_ImageSensor();
	UART_PRINT("Wake \n\r");
	//ExitStandby_mt9d111();
	Wakeup_ImageSensor();

	//		PCLK_Rate_read();

	// Configure Sensor in Capture Mode
	UART_PRINT("Start Sensor\n\r");
	lRetVal = StartSensorInJpegMode();
	STOPHERE_ON_ERROR(lRetVal);

	UART_PRINT("I2C Camera config done\n\r");

	while(1)
	{
		EnterStandby_mt9d111();

		ExitStandby_mt9d111();

		lRetVal = StartSensorInJpegMode();
		STOPHERE_ON_ERROR(lRetVal);

		UtilsDelay(3*80000000/3);
	}

}


void Test_ImageSensConfigFromFlash()
{
	sl_Start(0,0,0);

	WriteConfigRegFile_toFlash();

	CamControllerInit();	// Init parallel camera interface of cc3200
									// since image sensor needs XCLK for
									//its I2C module to work

	UtilsDelay(24/3 + 10);	// Initially, 24 clock cycles needed by MT9D111
									// 10 is margin

	SoftReset_ImageSensor();

	CameraSensorInit_SettingsFromFlash();

	StartCapture_SettingsFromFlash();

	MAP_PRCMPeripheralReset(PRCM_CAMERA);
	MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);

	while(1)
	{
		CollectTxit_ImgTempRH();
		sl_Stop(0xFFFF);
	}

}

void Test_TimerWorking()
{
	//
	// Initialize timer and test working
	//
	InitializeTimer();
	StartTimer();
	UtilsDelay(80000000/3); //2 sec delay
	StopTimer();
	float_t fTestDuration;
	GetTimeDuration(&fTestDuration);
	UART_PRINT("\n\rUtilsDelay Duration: %f\n\r", fTestDuration);
}

void Test_HibernateSlwClkCtrWakeup()
{
	/*uint32_t ulHibWkSrc;
	ulHibWkSrc = MAP_PRCMHibernateWakeupCauseGet();
	UART_PRINT("Hib wake cause: %x\n\r", ulHibWkSrc);
	ulHibWkSrc = *((volatile unsigned long *)(0x4402F800 + 0x00000468));
	UART_PRINT("Hib wake cause: %x\n\r\n\r", ulHibWkSrc);

	if (ulHibWkSrc == PRCM_HIB_WAKEUP_CAUSE_SLOW_CLOCK)
	{
		UART_PRINT("Wake Source : Slow Clock Counter\n\r");
	}
	if (ulHibWkSrc == PRCM_HIB_WAKEUP_CAUSE_GPIO)
	{
		UART_PRINT("Wake Source : GPIO\n\r");
	}*/

	if (MAP_PRCMSysResetCauseGet() == PRCM_POWER_ON)
	{
		UART_PRINT("Reset on power on\n\r");
		uint64_t ullSlowCounterReturnVal1;
		ullSlowCounterReturnVal1 = MAP_PRCMSlowClkCtrGet();
		UART_PRINT("b hib%lld\n\r\n\r", ullSlowCounterReturnVal1);
		HIBernate(ENABLE_TIMER_WAKESOURCE, NULL, (10.0*(1.0/60.0)));
		//HIBernate(ENABLE_GPIO02_WAKESOURCE, HIGH_LEVEL, 0);
	//PRCMHIBRegRead(HIB3P3_BASE+HIB3P3_O_MEM_HIB_RTC_WAKE_EN);
	}
	if (MAP_PRCMSysResetCauseGet() == PRCM_HIB_EXIT)
	{
		UART_PRINT("Reset on Hibernate exit\n\r");
		/*ulHibWkSrc = MAP_PRCMHibernateWakeupCauseGet();
		UART_PRINT("Hib wake cause: %x\n\r", ulHibWkSrc);
		if (ulHibWkSrc == PRCM_HIB_WAKEUP_CAUSE_SLOW_CLOCK)
		{
			UART_PRINT("Wake Source : Slow Clock Counter\n\r");
		}
		if (ulHibWkSrc == PRCM_HIB_WAKEUP_CAUSE_GPIO)
		{
			UART_PRINT("Wake Source : GPIO\n\r");
		}*/

		if(GPIOPinRead(GPIOA0_BASE, GPIO_PIN_2)&GPIO_PIN_2)
		{
			UART_PRINT("Wake Source : GPIO\n\r");
		}
		else
		{
			UART_PRINT("Wake Source : Slow Clock Counter\n\r");
		}

		uint64_t ullSlowCounterReturnVal1;
		ullSlowCounterReturnVal1 = MAP_PRCMSlowClkCtrGet();
		UART_PRINT("a wakeup %lld\n\r", ullSlowCounterReturnVal1);

		UtilsDelay(10*80000000/6);

		ullSlowCounterReturnVal1 = MAP_PRCMSlowClkCtrGet();
		UART_PRINT("b hib %lld\n\r\n\r", ullSlowCounterReturnVal1);

		HIBernate(ENABLE_TIMER_WAKESOURCE|ENABLE_GPIO02_WAKESOURCE, HIGH_LEVEL, (10.0*(1.0/60.0)));
		//HIBernate(ENABLE_TIMER_WAKESOURCE, NULL, 20*(1/60));
		//HIBernate(ENABLE_GPIO02_WAKESOURCE, HIGH_LEVEL, 0);
	}
}
