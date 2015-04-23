
#include "parse_uploads.h"
#include "camera_app.h"
#include "string.h"
#include "app_common.h"

extern char* dataBuffer;

void retreiveImageIDfromHTTPResponse(uint8_t* pucParseImageUrl);


//******************************************************************************
//
//	This function makes Parse related initializations
//
//	param - none
//
//	return created ParseClient
//
//******************************************************************************
ParseClient InitialiseParse()
{
	ParseClient client;

	setDataBufferPointer(g_image_buffer,sizeof(g_image_buffer));
	client = parseInitialize(PARSE_APP_ID, PARSE_CLIENT_KEY);

	return client;
}

//*****************************************************************************
//
//	This function uploads image to Parse.com
//
//	param[in]	client
//	param[in]	pucImageFileName - Pointer to Flash file name of Image
//	param[out]	pucParseImageUrl - Pointer to unique ImageName sent by Parse
//
//	Description: Upload is done using POST request. Construction of the packet
//	is handled within. The function also gives the Unique ImageName for the
//	image posted.
//
//*****************************************************************************
void UploadImageToParse(ParseClient client,
							uint8_t* pucImageFileName,
							uint8_t* pucParseImageUrl)
{
	parseSendRequest(client,
						"POST",
						"/1/files/myPic1.jpg",
						pucImageFileName,
						NULL,
						image);

	retreiveImageIDfromHTTPResponse(pucParseImageUrl);
}

//*****************************************************************************
//
//	This function uploads device state / sensor data to Parse.com
//
//	param[in]	client
//	param[in]	sensorDataFileName - Pointer to Flash file name of Sensor Data
//
//	Description: Upload is done using POST request. Construction of the packet
//	is handled within.
//
//*****************************************************************************
void UploadSensorDataToParse(ParseClient client,
								uint8_t* sensorDataFileName)
{
	parseSendRequest(client,
						"POST",
						"/1/classes/DeviceState", //DeviceState is object name
						sensorDataFileName,
						NULL,
						jsonObject);
//	parseSendRequestInternal(client,
//							"POST",
//							"/1/classes/DeviceState",
//							sensorDataFileName,
//							NULL,
//							1,
//							jsonObject);
}


//******************************************************************************
//
//	This function retrives the Parse Allocated Anique ImageName from the HTTP
//	response sent by Parse in response to Image POST request
//
//	param[out]	pucParseImageUrl - pointer to unique ImageName character array
//
//	return none
//
//	The search is done starting from the end of the text
//
// 	Warning: Use this function only when you know that the HTTP response is in
//			the memory pointed by dataBuffer pointer
//
//******************************************************************************
void retreiveImageIDfromHTTPResponse(uint8_t* pucParseImageUrl)
{
	uint16_t ucResponseLen;
	uint8_t* pucImageIDStart, *pucImageIDEnd;
	uint16_t ucLength = 0;

	//
	//	Initialize pointers to the end of http response message
	//
	ucResponseLen = strlen(dataBuffer);
	pucImageIDStart = dataBuffer + ucResponseLen - 1;
	pucImageIDEnd = pucImageIDStart - 3; // -3 : ",} and CR characters

	//
	//	Traverse backward till '/' is found
	//
	while( strncmp((const uint8_t*)pucImageIDStart, "/", 1) )
	{
		pucImageIDStart--;
	}
	pucImageIDStart++;	// To exclude '/'

	//
	// Calculate length
	//
	ucLength = pucImageIDEnd - pucImageIDStart + 1;

	strncpy( (uint8_t *)pucParseImageUrl,
			 (const uint8_t *)pucImageIDStart,
			 ucLength);
}


//******************************************************************************
//
//	This function constructs the JSON DeviceStae object that is to be uploaded
//	to cloud server
//
//	param[in]	pucParseImageUrl - pointer to unique ImageName character array
//	param[in]	fTemp - Temperature
//	param[in]	fRH - Relative Humidity
//	param[out]	pucSensorDataTxt - pointer to location where the json object is
//							to be put
//
//	return none
//
//	String functions are used
//
//******************************************************************************

void ConstructDeviceStateObject(uint8_t* pucParseImageUrl,
									float_t fTemp,
									float_t fRH,
									uint8_t* pucSensorDataTxt)
{
	pucSensorDataTxt[0] = NULL;

	uint8_t ucCharConv[20];
	memset(ucCharConv, '0', 20);
	strncat(pucSensorDataTxt, "{\"deviceId\":\"", sizeof("{\"deviceId\":\""));
	strncat(pucSensorDataTxt,
					VCOGNITION_DEVICE_ID,
					sizeof("VCOGNITION_DEVICE_ID"));
	strncat(pucSensorDataTxt,
					"\",\"photo\":{\"name\":\"",
					sizeof("\",\"photo\":{\"name\":\""));
	strncat(pucSensorDataTxt, pucParseImageUrl, strlen(pucParseImageUrl));
	strncat(pucSensorDataTxt,
					"\",\"__type\":\"File",
					sizeof("\",\"__type\":\"File"));
	strncat(pucSensorDataTxt, "\"},\"temp\":", sizeof("\"},\"temp\":"));
	intToASCII((uint16_t)(fTemp*100), ucCharConv);
	strncat(pucSensorDataTxt, ucCharConv, 2);
	strncat(pucSensorDataTxt, ".", 1);
	strncat(pucSensorDataTxt, ucCharConv + 2, 2);
	strncat(pucSensorDataTxt, ",\"humidity\":", sizeof(",\"humidity\":"));
	intToASCII((uint16_t)(fRH*100), ucCharConv);
	strncat(pucSensorDataTxt, ucCharConv, 2);
	strncat(pucSensorDataTxt, ".", 1);
	strncat(pucSensorDataTxt, ucCharConv+2, 2);
	strncat(pucSensorDataTxt, "}", sizeof("}"));
	UART_PRINT("\n%s\n", pucSensorDataTxt);
}
