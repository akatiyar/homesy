/*
 * flash_fileOperations.h
 *
 *  Created on: 08-Apr-2015
 *      Author: Chrysolin
 */

#ifndef FLASH_FILEOPERATIONS_H_
#define FLASH_FILEOPERATIONS_H_

#include <stdint.h>

#define	SINGLE_WRITE				1
#define MULTIPLE_WRITE_FIRST		2
#define MULTIPLE_WRITE_LAST			3
#define MULTIPLE_WRITE_MIDDLE		4

#define IMAGE_HEADER_FILE_NAME			"www/images/jpeg_header.jpg"
//#define IMAGE_DATA_FILE_NAME			"www/images/jpeg_image.jpg"
#define IMAGE_DATA_FILE_NAME			"www/images/cc3200_camera_capture.jpg"

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
#define FILESZ_ADC081C021				0
#define FILESZ_FXOS8700					32
#define FILESZ_ISL29035					160
#define FILESZ_SI7020					192
#define FILESZ_TCA6408A					224

#define OFFSET_ADC081C021				0
#define OFFSET_FXOS8700					(OFFSET_ADC081C021 + FILESZ_ADC081C021)
#define OFFSET_ISL29035					(OFFSET_FXOS8700 + FILESZ_FXOS8700)
#define OFFSET_SI7020					(OFFSET_ISL29035 + FILESZ_ISL29035)
#define OFFSET_TCA6408A					(OFFSET_SI7020 + FILESZ_SI7020)


#define FILENAME_SENSORCONFIGS			"sensor_configs_file"
#define MAX_FILESIZE_SENSORCONFIGS		1024	//Bytes
											//File created for this size
											//Can be used when more fields
											//are needed in the file
#define CONTENTSIZE_FILE_SENSORCONFIGS	(500)	//Bytes
											//Find way to employ the Macro


#define FILENAME_IMAGESENS_CONFIG	"imagesensor_configs_file"
#define MAX_FILESIZE_IMAGESENS_CONFIG					1024	//Bytes
											//File created for this size
											//Can be used when more fields
											//are needed in the file
#define CONTENTSIZE_FILE_IMAGESENS_CONFIG	1024	//Bytes


#define MAGN_INIT_VALS_FILE_NAME		"Magnetometer_Initial_Vals"
#define MAGN_INIT_VALS_FILE_MAXSIZE		20

#define FILE_NAME_USER_CONFIG			"config/user_config_file.txt"
#define FILE_SIZE_USER_CONFIG			1024



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
