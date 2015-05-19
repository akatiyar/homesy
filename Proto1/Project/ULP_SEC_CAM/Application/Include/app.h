

#ifndef GENERAL_INCLUDES_H_
#define GENERAL_INCLUDES_H_

#include <stdint.h>
#include <stdio.h>

#ifndef NOTERM
#include "uartA1_if.h"
#endif
#include "common.h"
#include "hw_types.h"
#include "gpio.h"
#include "hw_memmap.h"
#include "utils.h"
#include "rom_map.h"

#define APP_SSID_NAME 			"Solflr3"
#define APP_SSID_PASSWORD		"37203922bb"
#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

//#define APP_SSID_NAME 		"CAMERA"
//#define APP_SSID_PASSWORD		"37203922bb"
//#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

//#define APP_SSID_NAME 		"Chrysolin-test"
//#define APP_SSID_PASSWORD		"chrysolin"
//#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

#define VCOGNITION_DEVICE_ID	"fd1234cam3"

#define SLEEP_TIME              8000000
//#define OSI_STACK_SIZE          3000
#define OSI_STACK_SIZE          3000

#define IMAGE_QUANTIZ_SCALE		(0x0020)
//#define IMAGE_QUANTIZ_SCALE		(0x0009)	//Debug

#define FN_SUCCESS	0
#define FN_FAILED	1 //Funtion return values are uint16_t

#define SYSTEM_CLOCK	80000000

#define TEST_MODULES_INCLUDE
#define USB_DEBUG
#define DEBUG_ENABLE
#ifndef DEBUG_ENABLE
#define NOTERM
#endif

typedef enum
{
	APP_MODE_A,
	APP_MODE_B
}appModes;


// Application specific status/error codes
typedef enum
{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    CAMERA_CAPTURE_FAILED = -0x7D0,

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

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


#define STOPHERE_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	 while(1);\
					}\
            }




#endif /* GENERAL_INCLUDES_H_ */
