

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
#include "Error_codes.h"

#define FIRMWARE_VERSION 		"Release 0.0.19"
//#define FIRMWARE_VERSION 		"Uthra Testing 0.25"
//#define FIRMWARE_VERSION 		"F 0.38"
//#define FIRMWARE_VERSION 		"OTA through App 0.4"

//#define APP_SSID_NAME 			"Camera"
//#define APP_SSID_PASSWORD		"abcdef1234"
//#define APP_SSID_SEC_TYPE		SL_SEC_TYPE_WPA_WPA2


#define IMAGE_QUANTIZ_SCALE					(0x0030)	//=48d
//#define IMAGE_QUANTIZ_SCALE					(0x0020)	//=32d
#define RETRIES_MAX_NETWORK					5
#define LUX_THRESHOLD						2			//in Lux
#define DOORCHECK_TIMEOUT_SEC				60			//in sec
#define BATTERY_LOW_THRESHOLD				5			//in percent


#define SYSTEM_CLOCK						80000000
#define SLEEP_TIME              			8000000
//#define OSI_STACK_SIZE          			3000
#define OSI_STACK_SIZE          			4500		//in bytes
#define OSI_STACK_SIZE_MAIN_TASK			5000		//in bytes
#define OSI_STACK_SIZE_USERCONFIG_TASK		4500		//in bytes

#define PI									(3.141592654F)

#define FN_SUCCESS							0
#define FN_FAILED							(-1) //Funtion return values are int16_t

//To be passed to UtilsDelay(). Delay will be more if parallel tasks are also running
#define NO_OF_OPS_PERCYCLE					4
#define ONESEC_NO_OF_CYCLES					(SYSTEM_CLOCK/NO_OF_OPS_PERCYCLE)

#define NO		0
#define YES		1
#define NEVER	-1	//This is used only for the current fix for reducing Image sensor set up time

typedef enum
{
	APP_MODE_A,
	APP_MODE_B
}appModes;

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

typedef enum
{
	DEVICE_STATE,
	GROUND_DATA,
	USER_CONFIGS,
	FIRMWARE_VER,
}Parse_ClassNames;

typedef enum
{
	READ_MAGNTMTRFILE_DONE = 1,
	MAGNTMTRINIT_DONE,
	READ_SENSORCONFIGFILE_DONE,
	CAMERAINIT_DONE,
}e_Task3_NotificationValues;

typedef enum
{
	IMAGEFILE_OPEN_BEGUN = 1,
	IMAGEFILE_OPEN_COMPLETE,
	MAGNETOMETERINIT_STARTED,
	TIMERS_DISABLED,

}e_Task1_NotificationValues;

#ifdef NOTERM
#define DEBG_PRINT(x,...)
#define RELEASE_PRINT(x,...)
#define PRINT_ERROR(x)
#else	//NOTERM

#ifdef RELEASE_PRINT_ENABLE
#define RELEASE_PRINT  Report
#else	//RELEASE_PRINT_ENABLE
#define RELEASE_PRINT(x,...)
#endif	//RELEASE_PRINT_ENABLE

#ifdef DEBUG_PRINT_ENABLE
#define DEBG_PRINT  Report
#define PRINT_ERROR(x) Report("Err[%d] at line[%d] in fn[%s]\n",x,__LINE__,__FUNCTION__)
#else	//DEBUG_PRINT_ENABLE
#define DEBG_PRINT(x,...)
#define PRINT_ERROR(x)
#endif	//DEBUG_PRINT_ENABLE

#endif	//NOTERM

#define STOPHERE_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	g_latest_error = error_code;\
                	 	PRINT_ERROR(error_code);\
                	 	while(1);\
					}\
            }

#define RESET_CC3200_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	g_latest_error = error_code;\
                	 	PRINT_ERROR(error_code);\
                	 	PRCMSOCReset();\
					}\
            }

#define PRINT_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	g_latest_error = error_code;\
                	 	PRINT_ERROR(error_code);\
                 }\
            }

#define RETURN_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                	 	g_latest_error = error_code;\
                	 	PRINT_ERROR(error_code);\
                        return error_code;\
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
struct u64_time g_Struct_TimeStamp_MaxAngle;
struct u64_time g_Struct_TimeStamp_MinAngle;
struct u64_time g_Struct_TimeStamp_SnapAngle;
struct u64_time g_Struct_TimeStamp_OpenAngle;
float_t g_fBatteryVolt_atStart;
float_t g_fBatteryVolt_atEnd;
uint64_t g_TimeStamp_cc3200Up;
uint64_t g_TimeStamp_NWPUp;
uint64_t g_TimeStamp_CamUp;
uint64_t g_TimeStamp_PhotoSnap;
uint64_t g_TimeStamp_PhotoUploaded;
uint64_t g_TimeStamp_DoorClosed;
uint64_t g_TimeStamp_MinAngle;
uint64_t g_TimeStamp_MaxAngle;
uint64_t g_TimeStamp_SnapAngle;
uint64_t g_TimeStamp_OpenAngle;
int32_t g_latest_error;
int16_t g_fMaxAngle;
int16_t g_fMinAngle;
int16_t g_RawMaxAngle;
int16_t g_RawMinAngle;
uint8_t g_ucReasonForFailure;

#endif /* GENERAL_INCLUDES_H_ */
