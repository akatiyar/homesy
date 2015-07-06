/*
 * ota_updata.h
 *
 *  Created on: 03-Jul-2015
 *      Author: Chrysolin
 */

#ifndef APPLICATION_INCLUDE_OTA_H_
#define APPLICATION_INCLUDE_OTA_H_

#include "app.h"

#define OTA_VENDOR_STRING               "VCOG_DEV_001"	//Flashed
//#define OTA_VENDOR_STRING               "VCOG_DEV_002"

typedef enum
{
	DO_OTA_COMMAND = 0,
	ADD_HERE
}OTA_Status;

int32_t OTA_Update();
int32_t OTA_CommitImage();

#endif /* APPLICATION_INCLUDE_OTA_H_ */
