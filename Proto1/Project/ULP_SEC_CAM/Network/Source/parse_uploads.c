
#include "parse_uploads.h"
#include "camera_app.h"
#include "string.h"
#include "app_common.h"
#include "netcfg.h"
#include "flash_files.h"

#define GROUND_DATA_OBJECT_SIZE		1024	//bytes
#define USER_CONFIG_OBJECT_SIZE		500	//bytes
#define FIRMWARE_VER_OBJECT_SIZE	500	//bytes


//Ground Data : Field Names
#define DEVICE_ID					"deviceID"
#define FAILURE_REASON				"Failure_Reason"
#define TS_CC3200UP					"TimeStamp_CC3200Up"
#define TS_NWPUP					"TimeStamp_NWPUp"
#define TS_CAMUP					"TimeStamp_CamUp"
#define TS_PHOTOSNAP				"TimeStamp_PhotoSnapd"
#define TS_PHOTOUPLOAD				"TimeStamp_PhotoUped"
#define TS_DOORCLOSED				"TimeStamp_DoorClosed"
#define TS_MAXANGLE					"TimeStamp_MaxAngle"
#define TS_MINANGLE					"TimeStamp_MinAngle"
#define TS_SNAPANGLE				"TimeStamp_SnapAngle"
#define TS_OPENANGLE				"TimeStamp_OpenAngle"
#define MAX_ANGLE					"Max_DoorAngle"
#define MIN_ANGLE					"Min_DoorAngle"
#define RAWMAX_ANGLE				"RawMax_DoorAngle"
#define RAWMIN_ANGLE				"RawMin_DoorAngle"
#define ANGLE40						"Angle40"
#define ANGLE90						"Angle90"
#define ANGLE_OPEN					"AngleOpen"

#define MAGNETOMETER_CALIB_FITERROR	"MagnmtrCalib_FitError_Percent"

typedef enum{
	FIRST,
	MIDDLE,
	LAST
}e_FieldPosition;

extern char* dataBuffer;
extern float gdoor_90deg_angle;
extern float gdoor_40deg_angle;
extern float gdoor_OpenDeg_angle;
extern uint16_t g_shutterwidth;
extern uint16_t g_Gain[4];

char g_cResponseObjectID[OBJECT_ID_MAX_LEN];

int32_t retreiveImageIDfromHTTPResponse(uint8_t* pucParseImageUrl);
int simpleJsonProcessor(const char *data, const char *key, char* value, int size);

int32_t Add_NumberField_ToJSONString(uint8_t* pucGroundDataObject,
										uint8_t* pucFieldName,
										uint64_t ullFieldEntry,
										uint8_t ucFieldPosition);
int32_t ConstructGroundDataObject(uint8_t* pucFridgeCamID,
									uint8_t* pucGroundDataObject);

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
int32_t UploadImageToParse(ParseClient client,
							uint8_t* pucImageFileName,
							uint8_t* pucParseImageUrl)
{
	int32_t lRetVal;
	uint8_t ucTryNum;

	ucTryNum = 0;
	do{
		lRetVal = parseSendRequest(client,
										"POST",
										"/1/files/myPic1.jpg",
										(const char*)pucImageFileName,
										NULL,
										image);
		PRINT_ON_ERROR(lRetVal);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	memset(pucParseImageUrl, NULL, PARSE_IMAGE_URL_SIZE);
	simpleJsonProcessor(dataBuffer, "name",
							(char*) pucParseImageUrl, IMAGE_NAME_MAX_LEN);
	//retreiveImageIDfromHTTPResponse(pucParseImageUrl);

	return lRetVal;
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
int32_t UploadSensorDataToParse(ParseClient client, uint8_t* pucFridgeCamID,
								uint8_t* pucParseImageUrl, float_t fTemp,
								float_t fRH, uint8_t ucBatteryLvl,
								uint8_t* sensorDataObject)
{
	int32_t lRetVal;
	uint8_t ucTryNum;

	// Construct the JSON object string
	ConstructDeviceStateObject(pucFridgeCamID, pucParseImageUrl, fTemp, fRH,
								ucBatteryLvl, sensorDataObject);

	ucTryNum = 0;
	do{
		lRetVal = parseSendRequest(client,
							"POST",
							"/1/classes/DeviceState", //DeviceState is object name
							(const char*)sensorDataObject,
							NULL,
							jsonObject);
		PRINT_ON_ERROR(lRetVal);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	//Save the object ID in the global variable
	simpleJsonProcessor(dataBuffer, "objectId", g_cResponseObjectID,
								OBJECT_ID_MAX_LEN);

	//	parseSendRequestInternal(client,
	//							"POST",
	//							"/1/classes/DeviceState",
	//							sensorDataFileName,
	//							NULL,
	//							1,
	//							jsonObject);

	return lRetVal;
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
int32_t retreiveImageIDfromHTTPResponse(uint8_t* pucParseImageUrl)
{
	uint16_t ucResponseLen;
	uint8_t* pucImageIDStart, *pucImageIDEnd;
	uint16_t ucLength = 0;

	//
	//	Initialize pointers to the end of http response message
	//
	ucResponseLen = strlen(dataBuffer);
	pucImageIDStart = (uint8_t*)dataBuffer + ucResponseLen - 1;
	pucImageIDEnd = pucImageIDStart - 3; // -3 : ",} and CR characters

	//
	//	Traverse backward till '/' is found
	//
	while( strncmp((const char*)pucImageIDStart, "/", 1) )
	{
		pucImageIDStart--;
	}
	pucImageIDStart++;	// To exclude '/'

	//
	// Calculate length
	//
	ucLength = pucImageIDEnd - pucImageIDStart + 1;

	strncpy( (char *)pucParseImageUrl,
			 (const char *)pucImageIDStart,
			 ucLength);

	return 0;
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
int32_t ConstructDeviceStateObject(uint8_t* pucFridgeCamID,
									uint8_t* pucParseImageUrl,
									float_t fTemp,
									float_t fRH,
									uint8_t ucBatteryLevel,
									uint8_t* pucSensorDataTxt)
{
	uint8_t ucLength;
	uint8_t ucCharConv[20];

	memset(pucSensorDataTxt, '\0', DEVICE_STATE_OBJECT_SIZE);

	pucSensorDataTxt[0] = NULL;
	memset(ucCharConv, '0', 20);
	strncat((char*)pucSensorDataTxt, "{\"deviceId\":\"",
					sizeof("{\"deviceId\":\""));
//	strncat((char*)pucSensorDataTxt,
//					VCOGNITION_DEVICE_ID,
//					sizeof("VCOGNITION_DEVICE_ID"));
	strncat((char*)pucSensorDataTxt, (const char*)pucFridgeCamID,
					strlen((char*)pucFridgeCamID));
	strncat((char*)pucSensorDataTxt,
					"\",\"photo\":{\"name\":\"",
					sizeof("\",\"photo\":{\"name\":\""));
	strncat((char*)pucSensorDataTxt, (const char*)pucParseImageUrl,
					strlen((char*)pucParseImageUrl));
	strncat((char*)pucSensorDataTxt,
					"\",\"__type\":\"File",
					sizeof("\",\"__type\":\"File"));
	strncat((char*)pucSensorDataTxt, "\"},\"battery\":", sizeof("\"},\"battery\":"));
	ucLength = intToASCII((uint16_t)ucBatteryLevel, (char*)ucCharConv);
	strncat((char*)pucSensorDataTxt, (const char*)ucCharConv, ucLength);
	strncat((char*)pucSensorDataTxt, ",\"temp\":", sizeof("\"},\"temp\":"));
	intToASCII((uint16_t)(fTemp*100), (char*)ucCharConv);
	strncat((char*)pucSensorDataTxt, (const char*)ucCharConv, 2);
	strncat((char*)pucSensorDataTxt, ".", 1);
	strncat((char*)pucSensorDataTxt, (const char*)ucCharConv + 2, 2);
	strncat((char*)pucSensorDataTxt, ",\"humidity\":", sizeof(",\"humidity\":"));
	intToASCII((uint16_t)(fRH*100), (char*)ucCharConv);
	strncat((char*)pucSensorDataTxt, (const char*)ucCharConv, 2);
	strncat((char*)pucSensorDataTxt, ".", 1);
	strncat((char*)pucSensorDataTxt, (const char*)ucCharConv+2, 2);
	strncat((char*)pucSensorDataTxt, "}", sizeof("}"));

	RELEASE_PRINT("SensorDataObj:\n%s\n", pucSensorDataTxt);

	return 0;
}

//*****************************************************************************
//
//	This function uploads Ground Data Object to Parse.com
//
//	param[in]	client
//	param[in]	pucFridgeCamID - Pointer to FridgeCamID string
//
//	Description: Upload is done using POST request. Construction of the packet
//	is handled within.
//
//*****************************************************************************
int32_t UploadGroundDataObjectToParse(ParseClient client, uint8_t* pucFridgeCamID)
{
	int32_t lRetVal = -1;
	uint8_t ucTryNum;
	uint8_t ucGroundDataObject[GROUND_DATA_OBJECT_SIZE];

	// Construct the JSON object string
	ConstructGroundDataObject(pucFridgeCamID, &ucGroundDataObject[0]);

 	ucTryNum = 0;
	do{
		lRetVal = parseSendRequest(client,
							"POST",
							"/1/classes/GroundData", //GroundData is object name
							(const char*)ucGroundDataObject,
							NULL,
							jsonObject);
		PRINT_ON_ERROR(lRetVal);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//
//	This function constructs the JSON GroundData object that is to be uploaded
//	to cloud server (Parse.com)
//
//	param[out]	pucGroundDataObject - pointer to location where the json object
//							is to be put
//
//	return none
//
//	String functions are used
//
//******************************************************************************
int32_t ConstructGroundDataObject(uint8_t* pucFridgeCamID,
									uint8_t* pucGroundDataObject)
{
	char ObjectID[OBJECT_ID_MAX_LEN];
	memset(ObjectID, '\0', OBJECT_ID_MAX_LEN);

	memset(pucGroundDataObject, '\0', GROUND_DATA_OBJECT_SIZE);
	strncat((char*)pucGroundDataObject, "{\"deviceId\":\"",
					sizeof("{\"deviceId\":\""));
	strncat((char*)pucGroundDataObject, (const char*)pucFridgeCamID,
					strlen((char*)pucFridgeCamID));
	strncat((char*)pucGroundDataObject,	"\",", sizeof("\","));

	if(g_ucReasonForFailure == SUCCESS)
	{
		DEBG_PRINT("Success Reason Failure\n");
		//simpleJsonProcessor(dataBuffer, "objectId", ObjectID, OBJECT_ID_MAX_LEN);
		strcpy(ObjectID,g_cResponseObjectID);
	}
	else
	{
		memset(ObjectID, 'x', 5);
	}
	strncat((char*)pucGroundDataObject, "\"DevObjectId\":\"",
					sizeof("\"DevObjectId\":\""));
	strncat((char*)pucGroundDataObject, (const char*)ObjectID,
					strlen((char*)ObjectID));
	strncat((char*)pucGroundDataObject,	"\",", sizeof("\","));

	Add_NumberField_ToJSONString(pucGroundDataObject, FAILURE_REASON,
									g_ucReasonForFailure, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_CC3200UP,
									g_TimeStamp_cc3200Up, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_NWPUP,
									g_TimeStamp_NWPUp, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_CAMUP,
									g_TimeStamp_CamUp, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_PHOTOSNAP,
									g_TimeStamp_PhotoSnap, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_PHOTOUPLOAD,
									g_TimeStamp_PhotoUploaded, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_DOORCLOSED,
									g_TimeStamp_DoorClosed, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_MAXANGLE,
									g_TimeStamp_MinAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_MINANGLE,
									g_TimeStamp_MaxAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_SNAPANGLE,
									g_TimeStamp_SnapAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, TS_OPENANGLE,
									g_TimeStamp_OpenAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, MAX_ANGLE,
									g_fMaxAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, MIN_ANGLE,
									g_fMinAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, RAWMAX_ANGLE,
									g_RawMaxAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, RAWMIN_ANGLE,
									g_RawMinAngle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, ANGLE40,
									(long long)gdoor_40deg_angle, MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, ANGLE90,
									(long long)gdoor_90deg_angle,MIDDLE);
	Add_NumberField_ToJSONString(pucGroundDataObject, ANGLE_OPEN,
									(long long)gdoor_OpenDeg_angle,LAST);

	RELEASE_PRINT("GroundDataObj:\n%s\n", pucGroundDataObject);

	return 0;
}
//*****************************************************************************
//
//	This function uploads Ground Data Object to Parse.com
//
//	param[in]	client
//	param[in]	pucFridgeCamID - Pointer to FridgeCamID string
//
//	Description: Upload is done using POST request. Construction of the packet
//	is handled within.
//
//*****************************************************************************
int32_t UploadUserConfigObjectToParse(ParseClient client, uint8_t* pucFridgeCamID)
{
	int32_t lRetVal = -1;
	uint8_t ucTryNum;
	uint8_t ucUserConfigObject[USER_CONFIG_OBJECT_SIZE];

	// Construct the JSON object string
	ConstructUserConfigObject(pucFridgeCamID, &ucUserConfigObject[0]);

 	ucTryNum = 0;
	do{
		lRetVal = parseSendRequest(client,
							"POST",
							"/1/classes/UserConfigs", //UserConfigs is object name
							(const char*)ucUserConfigObject,
							NULL,
							jsonObject);
		PRINT_ON_ERROR(lRetVal);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//
//	This function constructs the JSON GroundData object that is to be uploaded
//	to cloud server (Parse.com)
//
//	param[out]	pucGroundDataObject - pointer to location where the json object
//							is to be put
//
//	return none
//
//	String functions are used
//
//******************************************************************************
int32_t ConstructUserConfigObject(uint8_t* pucFridgeCamID,
									uint8_t* pucUserConfigObject)
{
	int32_t lRetVal;
	float_t fUserConfigData[MAGNETOMETER_DATA_SIZE/(sizeof(float))];
	char Gain[10];
	uint8_t Cnt;

	lRetVal = ReadFile_FromFlash(((uint8_t*)fUserConfigData),
												(uint8_t*)USER_CONFIGS_FILENAME,
												MAGNETOMETER_DATA_SIZE,
												0);
	PRINT_ON_ERROR(lRetVal);

	memset(pucUserConfigObject, '\0', USER_CONFIG_OBJECT_SIZE);

	strncat((char*)pucUserConfigObject, "{\"deviceId\":\"",
					sizeof("{\"deviceId\":\""));
	strncat((char*)pucUserConfigObject, (const char*)pucFridgeCamID,
					strlen((char*)pucFridgeCamID));
	strncat((char*)pucUserConfigObject,	"\",", sizeof("\","));

	strncat((char*)pucUserConfigObject, "\"FirmwareVersion\":\"",
					sizeof("\"FirmwareVersion\":\""));
	strncat((char*)pucUserConfigObject, (const char*)FIRMWARE_VERSION,
					strlen((char*)FIRMWARE_VERSION));
	strncat((char*)pucUserConfigObject,	"\",", sizeof("\","));

	Add_NumberField_ToJSONString(pucUserConfigObject, MAGNETOMETER_CALIB_FITERROR,
									(long long)fUserConfigData[(OFFSET_FIT_ERROR/sizeof(float))], MIDDLE);
	Add_NumberField_ToJSONString(pucUserConfigObject, ANGLE40,
									(long long)fUserConfigData[(OFFSET_ANGLE_40/sizeof(float))], MIDDLE);
	Add_NumberField_ToJSONString(pucUserConfigObject, ANGLE90,
									(long long)fUserConfigData[(OFFSET_ANGLE_90/sizeof(float))],MIDDLE);
	Add_NumberField_ToJSONString(pucUserConfigObject, "Shutter_Width",g_shutterwidth,MIDDLE);

	strncat((char*)pucUserConfigObject, "\"Gain\":\"",
					sizeof("\"Gain\":\""));
	for(Cnt=0;Cnt<4;Cnt++)
	{
		memset(Gain, '\0', sizeof(Gain));
		intToASCII(g_Gain[Cnt],Gain);
		DEBG_PRINT("%s",Gain);
		strncat((char*)pucUserConfigObject, (const char*)Gain,
						strlen((char*)Gain));
		strncat((char*)pucUserConfigObject, "_",sizeof("_"));
	}

	strncat((char*)pucUserConfigObject,	"\",", sizeof("\","));

	Add_NumberField_ToJSONString(pucUserConfigObject, ANGLE_OPEN,
									(long long)fUserConfigData[(OFFSET_ANGLE_OPEN/sizeof(float))], LAST);

	RELEASE_PRINT("UserConfigObj:\n%s\n", pucUserConfigObject);

	return 0;
}
//******************************************************************************
//
// ullField entry data type is long long, but any lesser sized number can be passed
// ucFieldPosition - Position of the field in the JSOn string. Can be first, middle or last
//
//******************************************************************************
int32_t Add_NumberField_ToJSONString(uint8_t* pucGroundDataObject,
										uint8_t* pucFieldName,
										uint64_t ullFieldEntry,
										uint8_t ucFieldPosition)
{
	uint8_t ucCharConv[20];

	//Append { for first field only
	if(ucFieldPosition == FIRST)
	{
		strncat((char*)pucGroundDataObject, "{", sizeof("{"));
	}

	//Append "<FieldName>":
	strncat((char*)pucGroundDataObject, "\"", sizeof("\""));	//Append "
	strncat((char*)pucGroundDataObject, (const char*)pucFieldName,
						strlen((char*)pucFieldName));			//Append Field name
	strncat((char*)pucGroundDataObject, "\":", sizeof("\":"));	//Append ":

	//Append <FieldEntry>
	intToASCII(ullFieldEntry, (char*)ucCharConv);
	strncat((char*)pucGroundDataObject, (const char*)ucCharConv, sizeof(ucCharConv));

	//Append , only for first or middle fields
	if((ucFieldPosition == FIRST) || (ucFieldPosition == MIDDLE))
	{
		strncat((char*)pucGroundDataObject, ",", sizeof(","));
	}
	//Append } only for last field
	else if (ucFieldPosition == LAST)
	{
		strncat((char*)pucGroundDataObject, "}", sizeof("}"));
	}

	return 0;
}

//*****************************************************************************
//
//	This function uploads FirmwareVersion Object to Parse.com
//
//	param[in]	client
//	param[in]	pucFridgeCamID - Pointer to FridgeCamID string
//
//	Description: Upload is done using POST request. Construction of the packet
//	is handled within.
//
//*****************************************************************************
int32_t UploadFirmwareVersionObjectToParse(ParseClient client, uint8_t* pucFridgeCamID)
{
	int32_t lRetVal = -1;
	uint8_t ucTryNum;
	uint8_t ucFirmwareVerObject[FIRMWARE_VER_OBJECT_SIZE];

	// Construct the JSON object string
	strncat((char*)&ucFirmwareVerObject[0], "{\"deviceId\":\"",
					sizeof("{\"deviceId\":\""));
	strncat((char*)&ucFirmwareVerObject[0], (const char*)pucFridgeCamID,
					strlen((char*)pucFridgeCamID));
	strncat((char*)&ucFirmwareVerObject[0],	"\",", sizeof("\","));

	strncat((char*)&ucFirmwareVerObject[0], "\"FirmwareVersion\":\"",
					sizeof("\"FirmwareVersion\":\""));
	strncat((char*)&ucFirmwareVerObject[0], (const char*)FIRMWARE_VERSION,
					strlen((char*)FIRMWARE_VERSION));
	strncat((char*)&ucFirmwareVerObject[0],	"\"}", sizeof("\"}"));

	RELEASE_PRINT("FrimwareVerObj:\n%s\n", &ucFirmwareVerObject[0]);

 	ucTryNum = 0;
	do{
		lRetVal = parseSendRequest(client,
							"POST",
							"/1/classes/FirmwareVersion", //UserConfigs is object name
							(const char*)&ucFirmwareVerObject[0],
							NULL,
							jsonObject);
		PRINT_ON_ERROR(lRetVal);
		ucTryNum++;
	}while( (0 > lRetVal) && (RETRIES_MAX_NETWORK > ucTryNum) );
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}
