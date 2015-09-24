/*
 * get_fridge_cam_ID.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "app_common.h"
#include "netcfg.h"

#define FRIDGECAM_NAME_PREFIX		"Cam_"

//******************************************************************************
//	This function constructs and returns the Unique ID of the fridge cam
//
//	param[out]	pucFridgeCamID	- pointer to the variable where the DeviceID is
//								placed
//
//	return SUCCESS or FAILURE
//
//	The MAC ID of the device concatenated to 'Cam' is the device ID
//
//	Note: The function involves a simplelink call. So ensure NWP is on before
//			calling this fn.
//******************************************************************************
int32_t Get_FridgeCamID(uint8_t* pucFridgeCamID)
{
	int32_t lRetVal;
	uint8_t i;
	uint8_t* pucTemp;
	uint8_t macAddressVal[SL_MAC_ADDR_LEN];
	uint8_t macAddressLen = SL_MAC_ADDR_LEN;

	// Copy Prefix
	memset(pucFridgeCamID, '\0', FRIDGECAM_ID_SIZE);
	strcpy((char*)pucFridgeCamID, FRIDGECAM_NAME_PREFIX);

	// Read the unique MAC ID of the CC3200 device
	pucTemp = pucFridgeCamID + strlen(FRIDGECAM_NAME_PREFIX);
	lRetVal = sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(uint8_t *)macAddressVal);

	//Convert to 12-character array from 6-character array
	//Split the nibbles
	for(i=0; i<SL_MAC_ADDR_LEN; i++)
	{
		//Left Nibble
		*pucTemp = (macAddressVal[i] & 0xF0) >> 4;	//4-nibble size.
		pucTemp++;

		//RightNibble
		*pucTemp = (macAddressVal[i] & 0x0F);
		pucTemp++;
	}
	//Convert to ASCII
	pucTemp = pucFridgeCamID + strlen(FRIDGECAM_NAME_PREFIX) ;
	for(i=0; i<(SL_MAC_ADDR_LEN*2); i++)
	{
		//0-9
		if(*pucTemp <= 9)
		{
			*pucTemp = *pucTemp + 0x30; 		// 0(Char) = 0x30(ASCII)
		}
		//A-F
		else
		{
			*pucTemp = (*pucTemp-0x0A) + 0x41; // A(Char) = 0x41(ASCII)
		}
		pucTemp++;
	}

	DEBG_PRINT("FridgeCam ID: %s\n",pucFridgeCamID);
    return lRetVal;
}
