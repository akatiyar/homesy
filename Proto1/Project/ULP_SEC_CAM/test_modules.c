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

	MAP_PRCMPeripheralReset(PRCM_CAMERA);
	MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);

	while(1);
}
