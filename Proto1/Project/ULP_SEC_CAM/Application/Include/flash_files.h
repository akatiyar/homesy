/*
 * flash_fileOperations.h
 *
 *  Created on: 08-Apr-2015
 *      Author: Chrysolin
 */

#ifndef FLASH_FILEOPERATIONS_H_
#define FLASH_FILEOPERATIONS_H_

#include <stdint.h>

int32_t CreateFile_Flash(uint8_t* pucFileName, uint32_t uiMaxFileSize);
int32_t WriteFile_ToFlash(uint8_t* pucData, uint8_t* pucFileName);

#define MAGN_INIT_VALS_FILE_NAME		"Magnetometer_Initial_Vals"
#define MAGN_INIT_VALS_FILE_MAXSIZE		20


#endif /* FLASH_FILEOPERATIONS_H_ */
