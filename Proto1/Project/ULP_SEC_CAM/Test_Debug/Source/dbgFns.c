/*
 * dbg_Fns.c
 *
 *  Created on: 30-Jun-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "accelomtrMagntomtr_fxos8700.h"
#include "tempRHSens_si7020.h"
#include "lightSens_isl29035.h"
#include "appFns.h"
#include "mt9d111.h"
#include "camera_app.h"

#include "flash_files.h"
#include "fs.h"

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

// Give the file name inside the fn.
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
