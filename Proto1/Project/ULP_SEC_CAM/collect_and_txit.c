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

extern void CollectTxit_ImgTempRH();
//*****************************************************************************
//
//	The function captures image and sensor data and uploads to Parse
//
//	param none
//
//	return none
//
//*****************************************************************************
void CollectTxit_ImgTempRH()
{
#ifndef NO_CAMERA
	//
	//	Captute image and save in flash
	//
	CaptureAndStore_Image();
#endif
	//
	//	Connect cc3200 to wifi AP
	//
	ConfigureSimpleLinkToDefaultState();
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
	UploadImageToParse(clientHandle,
						(unsigned char*) USER_FILE_NAME,
						ucParseImageUrl);
	UART_PRINT("\n%s\n", ucParseImageUrl);

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

	//
	//	Upload sensor data to Parse
	//
	UploadSensorDataToParse(clientHandle, ucSensorDataTxt);

//	CollectSensorData();
//	txitSensorData(clientHandle,"",ucParseImageUrl);
}