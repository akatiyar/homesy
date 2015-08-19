#include "parse.h"

#include "app.h"
#include "math.h"

// MyApp
//#define PARSE_APP_ID		"InlZ2zle1OJi2f0Tfikz6DVhaz9IUdhHCzTW0f7w"
//#define PARSE_CLIENT_KEY	"KArqKTEjYrlT29hwW8p7aRYT1iTt1e9w6KS7g42E"

// homesy
#define PARSE_APP_ID		"FQHluCbjFceB6JYBIdiusFV7hVEW10N0n1lvVbxe"
#define PARSE_CLIENT_KEY	"2yFSV9m86ffL2cmcRwgRQvY6sa7nGViWpTv8QfHK"

#define DEVICE_STATE_OBJECT_SIZE	500	//bytes
#define PARSE_IMAGE_URL_SIZE		100	//bytes

ParseClient InitialiseParse();
int32_t UploadImageToParse(ParseClient client,
							uint8_t* ucImageDirName,
							uint8_t* ucParseImageName);
//int32_t UploadSensorDataToParse(ParseClient client,
//								uint8_t* sensorDataDirName);
int32_t UploadSensorDataToParse(ParseClient client, uint8_t* pucFridgeCamID,
								uint8_t* pucParseImageUrl, float_t fTemp,
								float_t fRH, uint8_t ucBatteryLvl,
								uint8_t* sensorDataObject);
int32_t ConstructDeviceStateObject(uint8_t* pucFridgeCamID,
									uint8_t* pucParseImageUrl,
									float_t fTemp,
									float_t fRH,
									uint8_t ucBatteryLevel,
									uint8_t* pucSensorDataTxt);
int32_t UploadGroundDataObjectToParse(ParseClient client,
										uint8_t* pucFridgeCamID);
int32_t ConstructUserConfigObject(uint8_t* pucFridgeCamID,
									uint8_t* pucUserConfigObject);
int32_t UploadUserConfigObjectToParse(ParseClient client,
										uint8_t* pucFridgeCamID);
int32_t UploadFirmwareVersionObjectToParse(ParseClient client,
										uint8_t* pucFridgeCamID);
