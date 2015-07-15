/*
 * collect_and_txit.c
 *
 *  Created on: 21-Apr-2015
 *      Author: Chrysolin
 */
#include "stdlib.h"

#include "app.h"
#include "camera_app.h"
#include "network_related_fns.h"
#include "parse_uploads.h"
#include "math.h"
#include "string.h"
#include "mt9d111.h"
#include "tempRHSens_si7020.h"

#include "flash_files.h"
#include "appFns.h"

extern uint8_t g_ucMagCalb;
int32_t CollectTxit_ImgTempRH();
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
	uint8_t ucTryNum = 0;
	float_t fTemp = 12.34, fRH = 56.78;
	uint8_t ucBatteryLvl = 80;
	uint8_t ucFridgeCamID[FRIDGECAM_ID_SIZE];

#ifndef NO_CAMERA
	//
	//	Captute image and save in flash
	//
	lRetVal = CaptureAndStore_Image();
	if((lRetVal == LIGHT_IS_OFF_BEFORE_IMAGING)||(lRetVal == TIMEOUT_BEFORE_IMAGING))
	{
		g_ulAppStatus = lRetVal;
		return lRetVal;
	}
	//SoftReset_ImageSensor();	//Including this since we are unable to standby image sensor while it is still running at times(?)
	Standby_ImageSensor();
	//CaptureinRAM_StoreAfterCapture_Image();
#endif

	//while(1);
	//
	//	Connect cc3200 to wifi AP
	//
	ConfigureSimpleLinkToDefaultState();
	//UtilsDelay(8000000);
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
										(unsigned char*) JPEG_IMAGE_FILE_NAME,
										ucParseImageUrl);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	PRINT_ON_ERROR(lRetVal);

	//
	//	Collect and Upload sensor data to Parse
	//
	if (lRetVal >= 0)	//Check whether image upload was successful
	{
		//
		//	Collect Temperature and RH values from Si7020 IC
		//
		getTempRH(&fTemp, &fRH);
		UART_PRINT("Temperature: %f\n\rRH: %f\n\r", fTemp, fRH);

		ucBatteryLvl = Get_BatteryPercent();

		Get_FridgeCamID(ucFridgeCamID);
		UART_PRINT("FridgeCam ID: %s\n\r",ucFridgeCamID);

		//
		// Construct the JSON object string
		//
		memset(ucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);
		ConstructDeviceStateObject(&ucFridgeCamID[0], &ucParseImageUrl[0], fTemp, fRH, ucBatteryLvl, &ucSensorDataTxt[0]);
		UART_PRINT("OBJECT: %s\n", ucSensorDataTxt);

		//
		//	Collect and Upload sensor data to Parse
		//
		ucTryNum = 0;
		do{
			lRetVal = UploadSensorDataToParse(clientHandle, ucSensorDataTxt);
			ucTryNum++;
		}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
		PRINT_ON_ERROR(lRetVal);
	}

	//	Free the memory allocated for clientHandle in InitialiseParse()
	free((void*)clientHandle);

	sl_Stop(0xFFFF);

	return lRetVal;
}
