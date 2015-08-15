/*
 * flash_fileOperations.h
 *
 *  Created on: 08-Apr-2015
 *      Author: Chrysolin
 */

#ifndef FLASH_FILEOPERATIONS_H_
#define FLASH_FILEOPERATIONS_H_

#include <stdint.h>

#define	SINGLE_WRITE						1
#define MULTIPLE_WRITE_FIRST				2
#define MULTIPLE_WRITE_LAST					3
#define MULTIPLE_WRITE_MIDDLE				4

#define FLASH_BLOCK_SIZE					4096
#define FLASH_FILE_HEADER_SIZE				440

//#define FILESIZE(X)\	//X is the number of KB needed. FILESIZE(X) returns the optimum file size. FILESIZE(X) >= X.
//			{\
//				if(((X*1024)%(FLASH_BLOCK_SIZE)) <= (FLASH_BLOCK_SIZE - FLASH_FILE_HEADER_SIZE)) \
//				{\
//					X = (X*1024 - FLASH_FILE_HEADER_SIZE);\
//				}\
//				else\
//				{\
//					X = ((X+4)*1024 - FLASH_FILE_HEADER_SIZE);\
//				}\
//			}

#define ONE_FLASH_BLOCK							((1*FLASH_BLOCK_SIZE) - FLASH_FILE_HEADER_SIZE)	//This is the minimum file size


#define JPEG_HEADER_FILE_NAME					"/data/jpeg_header"
#define JPEG_HEADER_MAX_FILESIZE				ONE_FLASH_BLOCK
//#define JPEG_IMAGE_FILE_NAME					"/data/jpeg_image"
#define JPEG_IMAGE_FILE_NAME					"www/jpeg_image.jpg"
//#define JPEG_IMAGE_MAX_FILESIZE				FILESIZE(150)
#define JPEG_IMAGE_MAX_FILESIZE					155208

#define USER_CONFIGS_FILENAME					"/config/user_configs"
#define USER_CONFIGS_MAX_FILESIZE				ONE_FLASH_BLOCK
//Section1 - Magnetometer configs
#define MAGNETOMETER_DATA_OFFSET				0
#define MAGNETOMETER_ALLOCATED_SPACE			0x80	//256 bytes
//Sub section i - Magn Calibs
#define OFFSET_MAG_CALB							(MAGNETOMETER_DATA_OFFSET)	// Intrms of Number of float values. This is an offset withing the Magnetometer section fo the file
#define SIZE_MAG_CALB							(12 * sizeof(float))
#define OFFSET_FIT_ERROR						(OFFSET_MAG_CALB+SIZE_MAG_CALB)
#define SIZE_SUBSECTION1						0x50	//80 bytes
//Sus section ii - Angles
#define OFFSET_ANGLE_90							(MAGNETOMETER_DATA_OFFSET + SIZE_SUBSECTION1)
#define SIZE_ANGLE_90							(sizeof(float))
#define OFFSET_ANGLE_40							(OFFSET_ANGLE_90 + SIZE_ANGLE_90)
#define SIZE_ANGLE_40							(sizeof(float))
#define OFFSET_ANGLE_OPEN						(OFFSET_ANGLE_40 + SIZE_ANGLE_40)
#define SIZE_ANGLE_OPEN							(sizeof(float))
#define MAGNETOMETER_DATA_SIZE					(OFFSET_ANGLE_OPEN + SIZE_ANGLE_OPEN)
#define SIZE_SUBSECTION2						0x50	//80 bytes. Can be increased up to 256-80 or a new sub-section can be had
//Section2 - WiFi configs
#define WIFI_DATA_OFFSET						(MAGNETOMETER_DATA_OFFSET + MAGNETOMETER_ALLOCATED_SPACE)
#define WIFI_DATA_SIZE							(AP_SSID_LEN_MAX + AP_PASSWORD_LEN_MAX + AP_SECTYPE_LEN_MAX)
#define CONTENT_LENGTH_USER_CONFIGS				(WIFI_DATA_OFFSET + WIFI_DATA_SIZE)

#define FILENAME_SENSORCONFIGS					"/configs/sensors_configs"
#define MAX_FILESIZE_SENSORCONFIGS				ONE_FLASH_BLOCK
#define FILESZ_ADC081C021						0
#define FILESZ_FXOS8700							32
#define FILESZ_ISL29035							160
#define FILESZ_SI7020							192
#define FILESZ_TCA6408A							224
#define FILESZ_MT9D111							1024
#define OFFSET_ADC081C021						0
#define OFFSET_FXOS8700							(OFFSET_ADC081C021 + FILESZ_ADC081C021)
#define OFFSET_ISL29035							(OFFSET_FXOS8700 + FILESZ_FXOS8700)
#define OFFSET_SI7020							(OFFSET_ISL29035 + FILESZ_ISL29035)
#define OFFSET_TCA6408A							(OFFSET_SI7020 + FILESZ_SI7020)
#define OFFSET_MT9D111							(OFFSET_TCA6408A + FILESZ_TCA6408A)
#define CONTENT_LENGTH_SENSORS_CONFIGS			(FILESZ_MT9D111 + OFFSET_MT9D111)



//#define FILENAME_ANGLE_VALS			"angle_values"	//Merge to "config/user_config"
//#define MAX_FILESIZE_ANGLE_VALS		1024	//Bytes
//											//File created for this size
//											//Can be used when more fields
//											//are needed in the file
//#define CONTENTSIZE_FILE_ANGLE_VALS	(50)	//Bytes
//											//Find way to employ the Macros



//#define FILENAME_USERWIFI			"user_wifi_profile_file"	//Merge to "config/user_config"
//#define MAX_FILESIZE_USERWIFI		1024	//Bytes
//											//File created for this size
//											//Can be used when more fields
//											//are needed in the file
//#define CONTENTSIZE_FILE_USERWIFI	(33+2+50)	//Bytes
//											//Find way to employ the Macros



//#define FILENAME_IMAGESENS_CONFIG		"/configs/mt9d111_configs"
//#define MAX_FILESIZE_IMAGESENS_CONFIG					1024	//Bytes
//											//File created for this size
//											//Can be used when more fields
//											//are needed in the file
//#define CONTENTSIZE_FILE_IMAGESENS_CONFIG	1024	//Bytes


//#define MAGN_INIT_VALS_FILE_NAME		"Magnetometer_Initial_Vals"	//Not used now
//#define MAGN_INIT_VALS_FILE_MAXSIZE		20
//
//#define FILE_NAME_USER_CONFIG			"config/user_config_file.txt"
//#define FILE_SIZE_USER_CONFIG			1024
//#define FILENAME_SENSORDATA			"sensor_data_file"	//Remane as "data/sensors_data"
//#define MAX_FILESIZE_SENSORDATA		1024	//Bytes
//											//File created for this size
//											//Can be used when more fields
//											//are needed in the file
//#define CONTENTSIZE_FILE_SENSORDATA	(500)	//Bytes
//											//Find way to employ the Macro


int32_t CreateFile_Flash(uint8_t* pucFileName, uint32_t uiMaxFileSize);
int32_t WriteFile_ToFlash(uint8_t* pucData,
			uint8_t* pucFileName,
			uint32_t uiDataSize,
			uint32_t uiOffsetInFile,
			uint8_t ucWriteType,
			int32_t* plFileHandle);
int32_t ReadFile_FromFlash(uint8_t* pucData,
							uint8_t* pucFileName,
							uint32_t uiDataSize,
							uint32_t uiOffsetInFile);


#endif /* FLASH_FILEOPERATIONS_H_ */
