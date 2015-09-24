/*
 * retreive_image_url.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "parse_uploads.h"

//******************************************************************************
//
//	This function retrives the Parse Allocated Anique ImageName from the HTTP
//	response sent by Parse in response to Image POST request
//
//	param[out]	pucParseImageUrl - pointer to unique ImageName character array
//
//	return none
//
//	The search is done starting from the end of the text
//
// 	Warning: Use this function only when you know that the HTTP response is in
//			the memory pointed by dataBuffer pointer
//
//******************************************************************************
int32_t retreiveImageIDfromHTTPResponse(uint8_t* pucParseImageUrl)
{
	uint16_t ucResponseLen;
	uint8_t* pucImageIDStart, *pucImageIDEnd;
	uint16_t ucLength = 0;

	//
	//	Initialize pointers to the end of http response message
	//
	ucResponseLen = strlen(dataBuffer);
	pucImageIDStart = (uint8_t*)dataBuffer + ucResponseLen - 1;
	pucImageIDEnd = pucImageIDStart - 3; // -3 : ",} and CR characters

	//
	//	Traverse backward till '/' is found
	//
	while( strncmp((const char*)pucImageIDStart, "/", 1) )
	{
		pucImageIDStart--;
	}
	pucImageIDStart++;	// To exclude '/'

	//
	// Calculate length
	//
	ucLength = pucImageIDEnd - pucImageIDStart + 1;

	strncpy( (char *)pucParseImageUrl,
			 (const char *)pucImageIDStart,
			 ucLength);

	return 0;
}
