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
	UART_PRINT("+++ I2C devices Debug +++\n\r");

	UART_PRINT("\nLightSens check...\n\r");
	verifyISL29035();

	UART_PRINT("\nA+M check...\n\r");
	verifyAccelMagnSensor();

	UART_PRINT("\nMT9D111 check...\n\r");
	CamControllerInit();	//Since Camera module is dependant on MCU for Clock
	UtilsDelay(24/3 + 10);
	Verify_ImageSensor();

	UART_PRINT("\nTempHumidity Sensor check...\n\r");
	verifyTempRHSensor();

	UART_PRINT("\nBatteryADC check...\n\r");
	Get_BatteryPercent();

	UART_PRINT("\nAll I2C devices READs successful\n\r");

	return 0;
}


int32_t Check_FlashFiles()
{
	uint32_t ulToken = 0;
	SlFsFileInfo_t FileInfo;
	int32_t lRetVal;

	sl_Start(0,0,0);
	UART_PRINT("%s\n\r", (uint8_t *)JPEG_IMAGE_FILE_NAME);
	lRetVal = sl_FsGetInfo((uint8_t *)JPEG_IMAGE_FILE_NAME, ulToken, &FileInfo);
	if(SL_FS_ERR_FILE_NOT_EXISTS == lRetVal)
	{
		UART_PRINT("Doesn't Exist");
	}
	else
	{
		UART_PRINT("%d\n\r%d\n\r", FileInfo.AllocatedLen, FileInfo.FileLen);
	}
	sl_Stop(0xffff);

	return 0;
}
