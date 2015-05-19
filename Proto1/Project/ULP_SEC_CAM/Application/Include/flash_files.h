/*
 * flash_fileOperations.h
 *
 *  Created on: 08-Apr-2015
 *      Author: Chrysolin
 */

#ifndef FLASH_FILEOPERATIONS_H_
#define FLASH_FILEOPERATIONS_H_

#include <stdint.h>

#define FILENAME_USERWIFI			"user_wifi_profile_file"
#define MAX_FILESIZE_USERWIFI		1024	//Bytes
											//File created for this size
											//Can be used when more fields
											//are needed in the file
#define CONTENTSIZE_FILE_USERWIFI	(33+2+50)	//Bytes
											//Find way to employ the Macros

#define FILENAME_SENSORDATA			"sensor_data_file"
#define MAX_FILESIZE_SENSORDATA		1024	//Bytes
											//File created for this size
											//Can be used when more fields
											//are needed in the file
#define CONTENTSIZE_FILE_SENSORDATA	(500)	//Bytes
											//Find way to employ the Macro


#define MAGN_INIT_VALS_FILE_NAME		"Magnetometer_Initial_Vals"
#define MAGN_INIT_VALS_FILE_MAXSIZE		20

#define FILE_NAME_USER_CONFIG			"config/user_config_file.txt"
#define FILE_SIZE_USER_CONFIG			1024



int32_t CreateFile_Flash(uint8_t* pucFileName, uint32_t uiMaxFileSize);
int32_t WriteFile_ToFlash(uint8_t* pucData,
							uint8_t* pucFileName,
							uint32_t uiDataSize,
							uint32_t uiOffsetInFile);
int32_t ReadFile_FromFlash(uint8_t* pucData,
							uint8_t* pucFileName,
							uint32_t uiDataSize,
							uint32_t uiOffsetInFile);


#endif /* FLASH_FILEOPERATIONS_H_ */
