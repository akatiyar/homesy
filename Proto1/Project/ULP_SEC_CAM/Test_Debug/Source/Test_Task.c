/*
 * Test_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "camera_app.h"

void Test_Task(void *pvParameters)
{


#ifdef COMPILE_THIS
	Config_And_Start_CameraCapture();

	Config_CameraCapture();
	Standby_ImageSensor();

	while(1)
	{
		Wakeup_ImageSensor();
		Start_CameraCapture();
		Standby_ImageSensor();
	}
#endif

#ifdef COMPIL_THIS
	sl_Start(0,0,0);

	uint8_t FridgeCamID[FRIDGECAM_ID_SIZE];
	Get_FridgeCamID(FridgeCamID);
	UART_PRINT("%s\n\r",FridgeCamID);
#endif
#ifdef COMPILE_THIS
	uint32_t ulTimeDuration_ms;
	start_100mSecTimer();
	MAP_UtilsDelay(0.3*80000000/6);
	//osi_Sleep(100);
	ulTimeDuration_ms = get_timeDuration();
	stop_100mSecTimer();
	UART_PRINT("%d ms\n\r", ulTimeDuration_ms);
#endif

#ifdef COMPILE_THIS
	uint32_t ulToken = 0;
	SlFsFileInfo_t FileInfo;
	int32_t lRetVal;

	sl_Start(0,0,0);
	sl_FsGetInfo((_u8*)JPEG_HEADER_FILE_NAME, ulToken, &FileInfo);
	lRetVal = ReadFile_FromFlash((uint8_t*)g_image_buffer, JPEG_HEADER_FILE_NAME, FileInfo.FileLen, 0);
	UART_PRINT("%d\n\r%d\n\r", FileInfo.FileLen, lRetVal);
	for(lRetVal = 0; lRetVal<(650/4);lRetVal++)
	{
		UART_PRINT("%x ", g_image_buffer[lRetVal]);
	}
	sl_Stop(0xFFFF);

	User_Configure();
	Config_And_Start_CameraCapture();
	CollectTxit_ImgTempRH();

	while(1);
#endif
#ifdef COMPILE_THIS
	Check_FlashFiles();
#endif
#ifdef COMPILE_THIS
	Check_I2CDevices();
	while(1)
	{
		UART_PRINT("%d\n\r", Get_BatteryPercent());
		UtilsDelay(80000000);
	}
#endif
#ifdef COMPILE_I2CDEVICE_CHECK
	Check_I2CDevices();
#endif

//#define COMPILE_THIS_1
#ifdef COMPILE_THIS_1
	uint8_t ucADCVal;
	while(1)
	{
		Get_BatteryVoltageLevel_ADC081C021(&ucADCVal);
		UART_PRINT("%xH\n\r", ucADCVal);
		UART_PRINT("%d\n\r", Get_BatteryPercent());
	}
#endif

#ifdef COMPILE_THIS
	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
#endif

#ifdef COMPILE_THIS
	UART_PRINT("%d\n",GPIOPinRead(GPIOA0_BASE, 0x04));
while(1)
{
	configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);
	getLightsensor_intrptStatus();
	UART_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	UART_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0

	configureISL29035(0, LUX_THRESHOLD, LIGHTOFF_TRIGGER);
	getLightsensor_intrptStatus();
	UART_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	UART_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0
}
#endif

#ifdef COMPILE_THIS
	while(1)
	{
		start_100mSecTimer();
		while(!(Elapsed_100MilliSecs == 100))
		{
		}
		stop_100mSecTimer();
	}
#endif

//	while(1)
//	{
//		InitializeTimer();
//		StartTimer();
//		UtilsDelay(5*80000000/6);
//	}

#ifdef COMPILE_THIS
	verifyISL29035();
	configureISL29035(0);
	uint16_t lux;
	while(1)
	{
		lux = getLightsensor_data();
		if(lux <= 5)
		{
			UART_PRINT("aaa\n");
		}
		if(IsLightOff(100))
		{
			UART_PRINT("ooo\n");
		}
	}
#endif

	//verifyISL29035();
	//verifyTempRHSensor();
	//verifyAccelMagnSensor();
	//Config_And_Start_CameraCapture();
	//Verify_ImageSensor();
//	FXOS8700_Init();
//	float fMagnFlux[3];
//	while(1)
//	{
//		UtilsDelay(0.25*80000000/6);
//		getMagnFlux_3axis(fMagnFlux);
//		UART_PRINT("%f, %f, %f\n\r", fMagnFlux[0], fMagnFlux[1], fMagnFlux[2]);
//	}
//	while(1)
//		fxos_main();
}
