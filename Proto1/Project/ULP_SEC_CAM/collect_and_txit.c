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
#include "tempRHSens_si7020.h"

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
	uint8_t ucSensorDataTxt[DEVICE_STATE_OBJECT_SIZE]; // DeviceState JSONobject
	ParseClient clientHandle;
	uint8_t ucParseImageUrl[100];
	//uint8_t ucParseImageUrl[] = "tfss-cdf82004-4241-425f-96ce-3f68092bebce-myPic1.jpg";
	uint8_t ucTryNum = 0;
	float_t fTemp = 12.34, fRH = 56.78;
	float_t fBatteryLvl = 80;

#ifndef NO_CAMERA
	//
	//	Captute image and save in flash
	//
	CaptureAndStore_Image();
	//CaptureinRAM_StoreAfterCapture_Image();
#endif

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
	clientHandle = InitialiseParse();

	//
	//	Upload image to Parse and retreive image's unique ID
	//
	memset(ucParseImageUrl, NULL, 100);
	do{
		lRetVal = UploadImageToParse(clientHandle,
										(unsigned char*) IMAGE_DATA_FILE_NAME,
										ucParseImageUrl);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);
	//UART_PRINT("Prakash-Line73 \n");

	//
	//	Collect Temperature and RH values from Si7020 IC
	//
	softResetTempRHSensor();
	configureTempRHSensor();
	UtilsDelay(80000000);
	getTempRH(&fTemp, &fRH);
	UART_PRINT("Temperature: %f\n\rRH: %f\n\r", fTemp, fRH);

//	Get_BatteryVoltageLevel_ADC081C021(&fBatteryLvl);

	//
	// Construct the JSON object string
	//
	memset(ucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);
	ConstructDeviceStateObject(ucParseImageUrl, fTemp, fRH, fBatteryLvl, ucSensorDataTxt);
	UART_PRINT("OBJECT: %s\n", ucSensorDataTxt);

	//
	//	Upload sensor data to Parse
	//
	ucTryNum = 0;
	do{
		lRetVal = UploadSensorDataToParse(clientHandle, ucSensorDataTxt);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	sl_Stop(0xFFFF);

//	CollectSensorData();
//	txitSensorData(clientHandle,"",ucParseImageUrl);

	return lRetVal;
}
