

#ifndef GENERAL_INCLUDES_H_
#define GENERAL_INCLUDES_H_

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#ifndef NOTERM
#include "uartA1_if.h"
#endif
#include "common.h"
#include "hw_types.h"
#include "gpio.h"
#include "hw_memmap.h"
#include "utils.h"
#include "rom_map.h"
#include "prcm.h"
#include "timer_if.h"
#include "rtc_hal.h"
#include "osi.h"

//#define FIRMWARE_VERSION 		"F30 Robust Angle"
//#define FIRMWARE_VERSION 		"F30 Reset with button"
#define FIRMWARE_VERSION 		"Release 0.0.13"
//#define FIRMWARE_VERSION 		"Uthra Testing 0.15"

//#define APP_SSID_NAME 		"Solflr3"
//#define APP_SSID_PASSWORD		"37203922bb"
//#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

#define APP_SSID_NAME 			"Camera"
#define APP_SSID_PASSWORD		"abcdef1234"
#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

//#define APP_SSID_NAME 		"Chrysolin-test"
//#define APP_SSID_PASSWORD		"chrysolin"
//#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

#define SLEEP_TIME              		8000000
//#define OSI_STACK_SIZE          		3000
#define OSI_STACK_SIZE          		3000	//in bytes
#define OSI_STACK_SIZE_MAIN_TASK			4000	//in bytes
#define OSI_STACK_SIZE_USERCONFIG_TASK		4000	//in bytes

#define IMAGE_QUANTIZ_SCALE		(0x0020)
//#define IMAGE_QUANTIZ_SCALE		(0x0009)	//Debug

#define RETRIES_MAX_NETWORK			5

#define FN_SUCCESS	0
#define FN_FAILED	1 //Funtion return values are uint16_t

#define SYSTEM_CLOCK	80000000

#define LUX_THRESHOLD					5

#define DOORCHECK_TIMEOUT_SEC			60
#define BATTERY_LOW_THRESHOLD			5	//percent
//#define BATTERY_LOW_THRESHOLD			32	//percent

#define PI				(3.141592654F)

#define TEST_MODULES_INCLUDE
//#define USB_DEBUG
#define DEBUG_ENABLE
#ifndef DEBUG_ENABLE
#define NOTERM
#endif

typedef enum
{
	APP_MODE_A,
	APP_MODE_B
}appModes;


// Application specific status/error codes. Assign these to lRetVal or g_appStatus
typedef enum
{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    CAMERA_CAPTURE_FAILED = -0x7D0,
	CAMERA_CONFIG_FAILED = CAMERA_CAPTURE_FAILED - 1,

    DEVICE_NOT_IN_STATION_MODE = CAMERA_CAPTURE_FAILED - 1,
    DEVICE_NOT_IN_AP_MODE = DEVICE_NOT_IN_STATION_MODE - 1,

    //File System Related errocodes
    FILE_OPEN_ERROR = DEVICE_NOT_IN_AP_MODE - 1,
    FILE_ALREADY_EXIST = FILE_OPEN_ERROR -1,
	FILE_CLOSE_ERROR = FILE_ALREADY_EXIST - 1,
	FILE_NOT_MATCHED = FILE_CLOSE_ERROR - 1,
	FILE_OPEN_READ_FAILED = FILE_NOT_MATCHED - 1,
	FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED -1,
	FILE_READ_FAILED = FILE_OPEN_WRITE_FAILED - 1,
	FILE_WRITE_FAILED = FILE_READ_FAILED - 1,

	//
	MT9D111_NOT_FOUND = FILE_WRITE_FAILED - 1,

	// WiFi Provisioning through AP Mode
	USER_WIFI_PROFILE_FAILED_TO_CONNECT = MT9D111_NOT_FOUND - 1,

	MT9D111_FIRMWARE_STATE_ERROR = USER_WIFI_PROFILE_FAILED_TO_CONNECT - 1,
	MT9D111_IMAGE_CAPTURE_FAILED = MT9D111_FIRMWARE_STATE_ERROR - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppErrorOrReturnCodes;

// Application specific status codes. Assign this to g_lAppStatus variable
//These are used for inter task communication
typedef enum
{
	START = 0,
	USER_CONFIG_TAKING_PLACE,
	USER_CONFIG_DONE,
	IMAGING_POSITION_DETECTED,
	LIGHT_IS_OFF_BEFORE_IMAGING,
	TIMEOUT_BEFORE_IMAGING,
	IMAGE_CAPTURED,
	IMAGESENSOR_CAPTURECONFIGS_HAPPENING,
	IMAGESENSOR_CAPTURECONFIGS_DONE,

}e_AppStatusCodes;

#define STOPHERE_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	 while(1);\
					}\
            }

#define RESET_CC3200_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	 PRCMSOCReset();\
					}\
            }

#define PRINT_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                        ERR_PRINT(error_code);\
                 }\
            }
//
//Global variable used through out the app
//
uint32_t g_ulAppStatus;
int8_t g_I2CPeripheral_inUse_Flag;
int8_t g_Task3_Notification;	//Can be set in Task3 or in other tasks.
								//Can be checked in Task3 or any other task.
								//For communication from Task3 or to Task3
int8_t g_Task1_Notification;
uint8_t g_flag_door_closing_45degree;
uint8_t g_ucFeedWatchdog;
uint32_t g_ulSimplelinkStatus;//SimpleLink Status
uint32_t g_ulAppTimeout_ms;

//Ground data in Parse
uint8_t g_ucReasonForFailure;
uint64_t g_TimeStamp_cc3200Up;
uint64_t g_TimeStamp_NWPUp;
uint64_t g_TimeStamp_CamUp;
uint64_t g_TimeStamp_PhotoSnap;
uint64_t g_TimeStamp_PhotoUploaded;
uint64_t g_TimeStamp_DoorClosed;
uint64_t g_TimeStamp_minAngle;
uint64_t g_TimeStamp_maxAngle;
int16_t g_fMaxAngle;
int16_t g_fMinAngle;
struct u64_time g_Struct_TimeStamp_MaxAngle;
struct u64_time g_Struct_TimeStamp_MinAngle;

typedef enum
{
	READ_MAGNTMTRFILE_DONE = 1,
	MAGNTMTRINIT_DONE,
}e_Task3_NotificationValues;

typedef enum
{
	IMAGEFILE_OPEN_BEGUN = 1,
	IMAGEFILE_OPEN_COMPLETE,

}e_Task1_NotificationValues;

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
	DOOR_SHUT_DURING_FILEOPEN,	//Image file open did not complete when snap position was detected
}e_GroundData_FailureReasonCodes;

#define NO		0
#define YES		1
#define NEVER	-1	//This is used only for the current fix for reducing Image sensor set up time

#endif /* GENERAL_INCLUDES_H_ */
