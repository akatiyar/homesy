/*
 * initialise_parse.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "parse_uploads.h"

//******************************************************************************
//
//	This function makes Parse related initializations
//
//	param - none
//
//	return created ParseClient
//
//******************************************************************************
ParseClient InitialiseParse()
{
	ParseClient client;

	setDataBufferPointer(g_image_buffer,sizeof(g_image_buffer));
	client = parseInitialize(PARSE_APP_ID, PARSE_CLIENT_KEY);

	return client;
}
