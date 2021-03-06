

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

#define APP_SSID_NAME 			"Solflr4"
#define APP_SSID_PASSWORD		"37203922bb"
#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2

#define VCOGNITION_DEVICE_ID	"fd1234cam3"

#define SLEEP_TIME              8000000
//#define OSI_STACK_SIZE          3000
#define OSI_STACK_SIZE          3000

#define FN_SUCCESS	0
#define FN_FAILED	1 //Funtion return values are uint16_t

//#define TEST_MODULES_INCLUDE
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

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;







#endif /* GENERAL_INCLUDES_H_ */
