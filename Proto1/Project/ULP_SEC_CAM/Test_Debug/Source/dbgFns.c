/*
 * dbg_Fns.c
 *
 *  Created on: 30-Jun-2015
 *      Author: Chrysolin
 */

#include <accelomtr_magntomtr_fxos8700.h>
#include <app_fns.h>
#include <light_sens_isl29035.h>
#include <temp_rh_sens_si7020.h>
#include "app.h"
#include "mt9d111.h"
#include "camera_app.h"

#include "flash_files.h"
#include "fs.h"

//******************************************************************************
//	Checks if the I2C devices are present. See UART messages for results.
//	I2C devices checked:
//		1. Light sensor
//		2. Accelerometer and magnetometer
//		3. Image Sensor
//		4. Temperature and RH sensor
//		5. ADC fo monitioring battery voltage
//******************************************************************************
int32_t Check_I2CDevices()
{
	DEBG_PRINT("\nLightSens...\n");
	verifyISL29035();

	DEBG_PRINT("\nA+M...\n");
	verifyAccelMagnSensor();

	DEBG_PRINT("\nMT9D111...\n");
	CamControllerInit();	//Since Camera module is dependant on MCU for Clock
	UtilsDelay(24/3 + 10);
	Verify_ImageSensor();

	DEBG_PRINT("\nTempHumidity...\n");
	verifyTempRHSensor();

	DEBG_PRINT("\nBatteryADC...\n");
	Get_BatteryPercent();

	return 0;
}

//******************************************************************************
//	Checks if a file is there in flash
// Give the file name inside the fn.
//******************************************************************************
int32_t Check_FlashFiles()
{
	uint32_t ulToken = 0;
	SlFsFileInfo_t FileInfo;
	int32_t lRetVal;

	sl_Start(0,0,0);
	DEBG_PRINT("%s\n\r", (uint8_t *)JPEG_IMAGE_FILE_NAME);
	lRetVal = sl_FsGetInfo((uint8_t *)JPEG_IMAGE_FILE_NAME, ulToken, &FileInfo);
	if(SL_FS_ERR_FILE_NOT_EXISTS == lRetVal)
	{
		DEBG_PRINT("Doesn't Exist");
	}
	else
	{
		DEBG_PRINT("%d\n\r%d\n\r", FileInfo.AllocatedLen, FileInfo.FileLen);
	}
	sl_Stop(0xffff);

	return 0;
}
