/*
 * jpeg.c
 *
 *  Created on: 22-Aug-2015
 *      Author: Chrysolin
 */

#include <nwp.h>
#include "app.h"
#include "flash_files.h"
#include "fs.h"
#include "jpeg.h"
#include "camera_app.h"

#define OFFSET_LUM_QTABLE_INHEADER				25
#define OFFSET_CHROM_QTABLE_INHEADER			(25+64+1)

void Get_LumQtable(uint8_t* Qtable, int32_t qscale);
void Get_ChromQtable(uint8_t* Qtable, int32_t qscale);

unsigned char JPEG_StdQuantTblY[64] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68,  109, 103, 77,
    24,  35,  55,  64,  81,  104, 113, 92,
    49,  64,  78,  87, 103,  121, 120, 101,
    72,  92,  95,  98, 112,  100, 103,  99
};

unsigned char JPEG_StdQuantTblC[64] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

//
// This table is used for regular-position to zigzagged-position lookup
//  This is Figure A.6 from the ISO/IEC 10918-1 1993 specification
//
static unsigned char zigzag[64] =
{
    0, 1, 5, 6,14,15,27,28,
    2, 4, 7,13,16,26,29,42,
    3, 8,12,17,25,30,41,43,
    9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63
};

//*****************************************************************************
//	Modify the Luminance and Chrominance quantization tables of the JPEG header
//	based no the specified image quantization scale
//
//	return	0 on success; error message on failure
//*****************************************************************************
int32_t Modify_JPEGHeaderFile()
{
	SlFsFileInfo_t fileInfo;
	int32_t lFileHandle;
	uint32_t ulToken = 0;
	long lRetVal;
	uint8_t* databuf;

	NWP_SwitchOn();

	sl_FsGetInfo((uint8_t*)JPEG_HEADER_FILE_NAME, ulToken, &fileInfo);
	lRetVal = ReadFile_FromFlash((uint8_t*)g_image_buffer, JPEG_HEADER_FILE_NAME, fileInfo.FileLen, 0);

	databuf = (uint8_t*)g_image_buffer;
	databuf += OFFSET_LUM_QTABLE_INHEADER;
	Get_LumQtable(databuf, IMAGE_QUANTIZ_SCALE);

	databuf = (uint8_t*)g_image_buffer;
	databuf += OFFSET_CHROM_QTABLE_INHEADER;
	Get_ChromQtable(databuf, IMAGE_QUANTIZ_SCALE);

	databuf = (uint8_t*)g_image_buffer;
	WriteFile_ToFlash(databuf, JPEG_HEADER_FILE_NAME, fileInfo.FileLen, 0, SINGLE_WRITE, &lFileHandle);

	ReadFile_FromFlash((uint8_t*)(g_image_buffer+1), JPEG_HEADER_FILE_NAME, fileInfo.FileLen, 0);

	NWP_SwitchOff();

	return lRetVal;
}

//*****************************************************************************
//	Calculate the Luminance quantization table based on the quantization scale
//
//	param	Qtable - pointer to the start of luminance quantization table
//	param	qscale - quantization scale
//*****************************************************************************
void Get_LumQtable(uint8_t* Qtable, int32_t qscale)
{
	int i, temp;
    // temp array to store scaled zigzagged quant entries
    unsigned char newtbl[64];

    // calculate scaled zigzagged luminance quantisation table entries
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblY[i] * qscale + 16) / 32;
        // limit the values to the valid range
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting luminance quant table to the output buffer
    for (i=0; i<64; i++)
        *Qtable++ = newtbl[i];
}

//*****************************************************************************
//	Calculate the chrominance quantization table based on the quantization
//	scale
//
//	param	Qtable - pointer to the start of chrominance quantization table
//	param	qscale - quantization scale
//*****************************************************************************
void Get_ChromQtable(uint8_t* Qtable, int32_t qscale)
{
	int i, temp;
    // temp array to store scaled zigzagged quant entries
    unsigned char newtbl[64];

    // calculate scaled zigzagged chrominance quantisation table entries
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblC[i] * qscale + 16) / 32;
        // limit the values to the valid range
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting chrominance quant table to the output buffer
    for (i=0; i<64; i++)
        *Qtable++ = newtbl[i];
}
