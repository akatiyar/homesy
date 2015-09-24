/*
 * add_number_field_to_json_string.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "string.h"
#include "parse_uploads.h"
#include "app_common.h"

//******************************************************************************
//
// ullField entry data type is long long, but any lesser sized number can be passed
// ucFieldPosition - Position of the field in the JSOn string. Can be first, middle or last
//
//******************************************************************************
int32_t Add_NumberField_ToJSONString(uint8_t* pucGroundDataObject,
										uint8_t* pucFieldName,
										uint64_t ullFieldEntry,
										uint8_t ucFieldPosition)
{
	uint8_t ucCharConv[20];

	//Append { for first field only
	if(ucFieldPosition == FIRST)
	{
		strncat((char*)pucGroundDataObject, "{", sizeof("{"));
	}

	//Append "<FieldName>":
	strncat((char*)pucGroundDataObject, "\"", sizeof("\""));	//Append "
	strncat((char*)pucGroundDataObject, (const char*)pucFieldName,
						strlen((char*)pucFieldName));			//Append Field name
	strncat((char*)pucGroundDataObject, "\":", sizeof("\":"));	//Append ":

	//Append <FieldEntry>
	intToASCII(ullFieldEntry, (char*)ucCharConv);
	strncat((char*)pucGroundDataObject, (const char*)ucCharConv, sizeof(ucCharConv));

	//Append , only for first or middle fields
	if((ucFieldPosition == FIRST) || (ucFieldPosition == MIDDLE))
	{
		strncat((char*)pucGroundDataObject, ",", sizeof(","));
	}
	//Append } only for last field
	else if (ucFieldPosition == LAST)
	{
		strncat((char*)pucGroundDataObject, "}", sizeof("}"));
	}

	return 0;
}
