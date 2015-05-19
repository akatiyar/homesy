/*
 * collect_and_txit.c
 *
 *  Created on: 21-Apr-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "camera_app.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "math.h"
#include "string.h"
#include "mt9d111.h"

#include "flash_files.h"
extern int32_t CollectTxit_ImgTempRH();
//*****************************************************************************
//
//	The function captures image and sensor data and uploads to Parse
//
//	param none
//
//	return none
//
//*****************************************************************************
int32_t CollectTxit_ImgTempRH()
{
	int32_t lRetVal;

#ifndef NO_CAMERA
	//
	//	Captute image and save in flash
	//
//	while(1)
		CaptureAndStore_Image();
#endif

//	PCLK_Rate_read();

//	ReadAllAEnAWBRegs();

	/*while(1)
	{
		//AnalogGainReg_Read();
		ReadAllAEnAWBRegs();

		disableAE();
		disableAWB();

		ReadAllAEnAWBRegs();

		enableAE();
		enableAWB();
	}*/

	//while(1);
	//
	//	Connect cc3200 to wifi AP
	//
	ConfigureSimpleLinkToDefaultState();
	UtilsDelay(8000000);
	lRetVal = sl_Start(0, 0, 0);
	ConnectToNetwork_STA();

	//
	//	Parse initialization
	//
	ParseClient clientHandle;
	clientHandle = InitialiseParse();

	//
	//	Upload image to Parse and retreive image's unique ID
	//
	uint8_t ucParseImageUrl[100];
	memset(ucParseImageUrl,NULL, 100);
	lRetVal = UploadImageToParse(clientHandle,
									(unsigned char*) USER_FILE_NAME,
									ucParseImageUrl);
	ASSERT_ON_ERROR(lRetVal);
	//UART_PRINT("\n%s\n", ucParseImageUrl);

	//
	//	Collect Temperature and RH values from Si7020 IC
	//
	float_t fTemp = 12.34, fRH = 56.78;
//	verifyTempRHSensor();
//	softResetTempRHSensor();
//	configureTempRHSensor();
//	UtilsDelay(80000000);
//	getTempRH(&fTemp, &fRH);

	//
	// Construct the JSON object string
	//
	uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	memset(ucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);
	ConstructDeviceStateObject(ucParseImageUrl, fTemp, fRH, ucSensorDataTxt);

	lRetVal = CreateFile_Flash(FILENAME_SENSORDATA, MAX_FILESIZE_SENSORDATA);
	ASSERT_ON_ERROR(lRetVal);

	int32_t lFileHandle;
	lRetVal = WriteFile_ToFlash((uint8_t*)&ucSensorDataTxt[0],
								(uint8_t*)FILENAME_SENSORDATA,
								CONTENTSIZE_FILE_SENSORDATA,
								0, SINGLE_WRITE,
								&lFileHandle);
	ASSERT_ON_ERROR(lRetVal);

	// For verification - DBG
	lRetVal = ReadFile_FromFlash((ucSensorDataTxt+3),
									(uint8_t*)FILENAME_SENSORDATA,
									CONTENTSIZE_FILE_SENSORDATA-3, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = ReadFile_FromFlash(ucSensorDataTxt,
									(uint8_t*)FILENAME_SENSORDATA,
									CONTENTSIZE_FILE_SENSORDATA, 0);
	ASSERT_ON_ERROR(lRetVal);


	//
	//	Upload sensor data to Parse
	//
	UploadSensorDataToParse(clientHandle, ucSensorDataTxt);

	//sl_Stop(0xFFFF);

//	CollectSensorData();
//	txitSensorData(clientHandle,"",ucParseImageUrl);

	return lRetVal;
}
