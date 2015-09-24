/*
 * Test_Task.c
 *
 *  Created on: 14-Jul-2015
 *      Author: Chrysolin
 */

#include <app_fns.h>
#include "app.h"
#include "camera_app.h"
#include "app_common.h"
#include "osi.h"
#include "timer_fns.h"
#include "timer_fns.h"

#include "fs.h"
#include "flash_files.h"

#include "nwp.h"


void Test_Task(void *pvParameters)
{
    long lRetVal;
    long lFileHandle;
	unsigned long ulToken = NULL;
	uint32_t ulTimeDuration_ms;

	NWP_SwitchOn();

	start_100mSecTimer();
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
					   FS_MODE_OPEN_WRITE, &ulToken, &lFileHandle);
	ulTimeDuration_ms = get_timeDuration();
	stop_100mSecTimer();
	DEBG_PRINT("First open: %dms\n", ulTimeDuration_ms);

	PRINT_ON_ERROR(lRetVal);
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	PRINT_ON_ERROR(lRetVal);

	start_100mSecTimer();
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
						   FS_MODE_OPEN_WRITE, &ulToken, &lFileHandle);
	ulTimeDuration_ms = get_timeDuration();
	stop_100mSecTimer();
	DEBG_PRINT("Second open: %dms\n", ulTimeDuration_ms);

	PRINT_ON_ERROR(lRetVal);
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	PRINT_ON_ERROR(lRetVal);

	lRetVal = sl_FsDel((unsigned char *)JPEG_IMAGE_FILE_NAME, ulToken);
	PRINT_ON_ERROR(lRetVal);
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,//0x00212001,
						//FS_MODE_OPEN_CREATE(65260,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						//FS_MODE_OPEN_CREATE(65,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
							//FS_MODE_OPEN_CREATE(120535,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						FS_MODE_OPEN_CREATE(153600,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
	                    &ulToken,
	                    &lFileHandle);
	PRINT_ON_ERROR(lRetVal);
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	PRINT_ON_ERROR(lRetVal);
	start_100mSecTimer();
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
						   FS_MODE_OPEN_WRITE, &ulToken, &lFileHandle);
	ulTimeDuration_ms = get_timeDuration();
	stop_100mSecTimer();
	DEBG_PRINT("Third open: %dms\n", ulTimeDuration_ms);
	PRINT_ON_ERROR(lRetVal);
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	PRINT_ON_ERROR(lRetVal);

	NWP_SwitchOff();

#if 0
	uint16_t lux_reading[50];
	int i;

	/*for(i=0; i<50; i++)
	{
		configureISL29035(0, NULL, NULL);
		lux_reading[i] = getLightsensor_data();
		UtilsDelay(.01*80000000/4);
	}*/

	for(i=0; i<50; i++)
	{
		configureISL29035(0, NULL, NULL);
		lux_reading[i] = getLightsensor_data();
		DEBG_PRINT("Lux: %d\n",lux_reading[i]);
		UtilsDelay(80000000/4);
	}
	while(1);
#endif

#if 0
	uint8_t data;
	FXOS8700_Init(NULL, NULL);
	i2cReadRegisters(0x1E, 0x5B, 1, &data);
	DEBG_PRINT("b standby: %x\n", data);
	reset_accelMagn_fxos8700();
	i2cReadRegisters(0x1E, 0x5B, 1, &data);
	DEBG_PRINT("a standby: %x\n", data);
	while(1);
#endif

#ifdef COMPLIE_THIS
	start_periodicInterrupt_timer(2.5);
	while(1)
	{
		DEBG_PRINT(".");
		osi_Sleep(1);
	}
#endif

#if 0
	g_fMinAngle = 361;
	g_fMaxAngle = 0;

	gdoor_90deg_angle = 316;
	gdoor_40deg_angle = 12;
	gdoor_OpenDeg_angle = 350;

	g_angleOffset_to180 = 180.0 - gdoor_40deg_angle;

	//float_t f[] = {20,15,10,5,0,355,360,355,350,345,340,335,330,325,320,315,310,305,300,295};
	float_t f[] = {330,0,355,20,15,315,325,340,335,360,305,300,295,10,345,355,320,5,310,350};
	int i;
	for(i=0; i<(sizeof(f)/sizeof(float_t)); i++)
	{
		Check_MinMax(f[i]);
		DEBG_PRINT("%d %3.2f\n", i, f[i]);
	}
	Calculate_TrueMinMaxAngles();
#endif

#if 0
	while(1)
	{
		DEBG_PRINT("%x\n",IS_PUSHBUTTON_PRESSED);
	}
#endif

#if 0
	float OpenAngle;

	OpenAngle = Calculate_DoorOpenThresholdAngle(30.56,80.32);
	DEBG_PRINT("Open Door Angle : %f\n",OpenAngle);
	OpenAngle = Calculate_DoorOpenThresholdAngle(80.32,30.56);
	DEBG_PRINT("Open Door Angle : %f\n",OpenAngle);

	OpenAngle = Calculate_DoorOpenThresholdAngle(330.83, 19.2);
	DEBG_PRINT("Open Door Angle : %f\n",OpenAngle);
	OpenAngle = Calculate_DoorOpenThresholdAngle(19.2, 330.83);
	DEBG_PRINT("Open Door Angle : %f\n",OpenAngle);
#endif

#if 0
	LEDTimer_Enable();
	LED_Blink_2(.2,.2,BLINK_FOREVER);
	int i = 100;
	while(i)
		{
			osi_Sleep(100);
			DEBG_PRINT("j");
			i--;
		}
	LED_Blink_2(.2,1,BLINK_FOREVER);
	i = 100;
	while(i)
	{
		osi_Sleep(100);
		DEBG_PRINT("k");
		i--;
	}
	LED_Blink_2(0,1,0);
	LED_Blink_2(1,0,0);
	LEDTimer_Disable();
	//LED_Blink_2(.2,,10);

	while(1)
	{
		osi_Sleep(100);
		DEBG_PRINT("j");
	}
	//LED_Blink_2(1,.5,5);
#endif

#if 0
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
	DEBG_PRINT("%s\n\r",FridgeCamID);
#endif
#if 0
	uint32_t ulTimeDuration_ms;
	start_100mSecTimer();
	MAP_UtilsDelay(0.3*80000000/6);
	//osi_Sleep(100);
	ulTimeDuration_ms = get_timeDuration();
	stop_100mSecTimer();
	DEBG_PRINT("%d ms\n\r", ulTimeDuration_ms);
#endif

#if 0
	uint32_t ulToken = 0;
	SlFsFileInfo_t FileInfo;
	int32_t lRetVal;

	sl_Start(0,0,0);
	sl_FsGetInfo((_u8*)JPEG_HEADER_FILE_NAME, ulToken, &FileInfo);
	lRetVal = ReadFile_FromFlash((uint8_t*)g_image_buffer, JPEG_HEADER_FILE_NAME, FileInfo.FileLen, 0);
	DEBG_PRINT("%d\n\r%d\n\r", FileInfo.FileLen, lRetVal);
	for(lRetVal = 0; lRetVal<(650/4);lRetVal++)
	{
		DEBG_PRINT("%x ", g_image_buffer[lRetVal]);
	}
	sl_Stop(0xFFFF);

	User_Configure();
	Config_And_Start_CameraCapture();
	CollectTxit_ImgTempRH();

	while(1);
#endif
#if 0
	Check_FlashFiles();
#endif
#if 0
	Check_I2CDevices();
	while(1)
	{
		DEBG_PRINT("%d\n\r", Get_BatteryPercent());
		UtilsDelay(80000000);
	}
#endif
#ifdef COMPILE_I2CDEVICE_CHECK
	Check_I2CDevices();
#endif

//#define COMPILE_THIS_1
#if 0
	uint8_t ucADCVal;
	while(1)
	{
		Get_BatteryVoltageLevel_ADC081C021(&ucADCVal);
		DEBG_PRINT("%xH\n\r", ucADCVal);
		DEBG_PRINT("%d\n\r", Get_BatteryPercent());
	}
#endif

#if 0
	Config_And_Start_CameraCapture();
	while(1)
	{
		CollectTxit_ImgTempRH();
	}
#endif

#if 0
	DEBG_PRINT("%d\n",GPIOPinRead(GPIOA0_BASE, 0x04));
while(1)
{
	configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);
	getLightsensor_intrptStatus();
	DEBG_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	DEBG_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0

	configureISL29035(0, LUX_THRESHOLD, LIGHTOFF_TRIGGER);
	getLightsensor_intrptStatus();
	DEBG_PRINT("Light supposedly on %d\n",GPIOPinRead(GPIOA0_BASE, 0x04));	//1
	while(GPIOPinRead(GPIOA0_BASE, 0x04));
	DEBG_PRINT("Light supposedly off %d\n",GPIOPinRead(GPIOA0_BASE, 0x04)); //0
}
#endif

#if 0
	while(1)
	{
		start_100mSecTimer();
		while(!(Elapsed_100MilliSecs == 100))
		{
		}
		stop_100mSecTimer();
	}
#endif

#if 0
//	while(1)
//	{
//		InitializeTimer();
//		StartTimer();
//		UtilsDelay(5*80000000/6);
//	}
#endif

#if 0
	verifyISL29035();
	configureISL29035(0);
	uint16_t lux;
	while(1)
	{
		lux = getLightsensor_data();
		if(lux <= 5)
		{
			DEBG_PRINT("aaa\n");
		}
		if(IsLightOff(100))
		{
			DEBG_PRINT("ooo\n");
		}
	}
#endif

#if 0
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
//		DEBG_PRINT("%f, %f, %f\n\r", fMagnFlux[0], fMagnFlux[1], fMagnFlux[2]);
//	}
//	while(1)
//		fxos_main();
#endif
}
