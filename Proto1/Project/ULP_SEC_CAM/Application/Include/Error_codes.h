/*
 * Error_codes.h
 *
 *  Created on: 07-Sep-2015
 *      Author: Chrysolin
 */

#ifndef APPLICATION_INCLUDE_ERROR_CODES_H_
#define APPLICATION_INCLUDE_ERROR_CODES_H_


// Application specific status/error codes. Assign these to lRetVal or g_appStatus
typedef enum
{
    // Choosing -0x7D0 = -2000 to avoid overlap w/ host-driver's error codes

	//I2C device not found
	MT9D111_NOT_FOUND = -2000,						//Image Sensor
	ISL29035_NOT_FOUND = MT9D111_NOT_FOUND - 1,		//Light Sensor
	FXOS8700_NOT_FOUND = ISL29035_NOT_FOUND - 1,	//Accel, Magnetometer
	SI7020_NOT_FOUND = FXOS8700_NOT_FOUND - 1,		//Temp RH
	ADC081C021_NOT_FOUND = SI7020_NOT_FOUND - 1,	//Battery ADC
	PCF8574_NOT_FOUND = ADC081C021_NOT_FOUND - 1,	//IO expander

	//Peripheral devices related error codes
	UNABLE_TO_RESET_FXOS8700 = PCF8574_NOT_FOUND - 1,

	//MT9D111 related error codes
	MT9D111_FIRMWARE_STATE_ERROR = UNABLE_TO_RESET_FXOS8700 - 1,
	MT9D111_IMAGE_CAPTURE_FAILED = MT9D111_FIRMWARE_STATE_ERROR - 1,
	MT9D111_STANDBY_FAILED = MT9D111_IMAGE_CAPTURE_FAILED - 1,

    //Network related error codes
	DEVICE_NOT_IN_STATION_MODE = MT9D111_STANDBY_FAILED - 1,
    DEVICE_NOT_IN_AP_MODE = DEVICE_NOT_IN_STATION_MODE - 1,
	USER_WIFI_PROFILE_FAILED_TO_CONNECT = DEVICE_NOT_IN_AP_MODE - 1,
									// WiFi Provisioning through AP Mode

    //File System Related error codes
    FILE_OPEN_ERROR = USER_WIFI_PROFILE_FAILED_TO_CONNECT - 1,
    FILE_ALREADY_EXIST = FILE_OPEN_ERROR -1,
	FILE_CLOSE_ERROR = FILE_ALREADY_EXIST - 1,
	FILE_NOT_MATCHED = FILE_CLOSE_ERROR - 1,
	FILE_OPEN_READ_FAILED = FILE_NOT_MATCHED - 1,
	FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED -1,
	FILE_READ_FAILED = FILE_OPEN_WRITE_FAILED - 1,
	FILE_WRITE_FAILED = FILE_READ_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppErrorOrReturnCodes;

//Reasons for failure
typedef enum
{
	NEVER_WENT_TO_ANGLECHECK = 1,	//Door shut before wake-up initializations were done (i.e door closed in < 1.5 sec)
	NOTOPEN_NOTCLOSED, 	//or equivalently, light went out. Likely reason: door opening was too narrow
	OPEN_NOTCLOSED,		//a likely problem with angle detection. Validate open and snap angles
	NOTOPEN_CLOSED,		//user opens door very narrowly, so open condition was not met but snap condition was met
	TIMEOUT_BEFORE_IMAGESNAP,	//Door open for too long
	IMAGE_NOTCAPTURED,			//Image capture failure due to some image sensor/camera peripheral issue
	IMAGE_NOTUPLOADED,			//Upload to Parse failed after 5 retries
	DOOR_ATSNAP_DURING_FILEOPEN,//Image file open did not complete when snap position was detected
}e_GroundData_FailureReasonCodes;

#endif /* APPLICATION_INCLUDE_ERROR_CODES_H_ */
