#include "app_common.h"
#include "math.h"
const char pcDigits[] = "0123456789"; /* variable used by itoa function */
#include "simplelink.h"
#include "LED_Timer.h"
#include "string.h"

#define FRIDGECAM_NAME_PREFIX		"Cam_"
//*****************************************************************************
//	This function has been taken from CC3200 SDK network_if.c
//! itoa
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters
//!
//!
//
//*****************************************************************************
unsigned short intToASCII(long long cNum, char *cString)	//Tag:CS
{
    char* ptr;
    //short uTemp = cNum;
    long long uTemp = cNum;		//Tag:CS
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';
        *++cString = '\0';
        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    *ptr = '\0';			//Tag:CS
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return (length+1);	//Tag:CS
}

//******************************************************************************
//	This function blinks the LED
//******************************************************************************
void LED_Blink(uint8_t ucHowManyTimes, float_t fSecsForEachCycle)
{
	uint8_t i;
	for(i=0;i<ucHowManyTimes;i++)
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
		UtilsDelay((fSecsForEachCycle/2.0)*80000000/6);
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
		UtilsDelay((fSecsForEachCycle/2.0)*80000000/6);
	}
}

void LED_On()
{
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	LED_Blink_2(1,0,0);
}

void LED_Off()
{
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
	LED_Blink_2(0,1,0);
}


//******************************************************************************
//Blink LED using timer - specifying different on and off times and the number
//of blinks
//******************************************************************************
void LED_Blink_2(float_t fOnTime_inSecs, float_t fOffTime_inSecs, int8_t ucHowManyTimes)
{
	LEDTimer_Stop();	//Stopping the Timer, in case it is already running
						//(ie previous LED blink is happening)

	//Keep the LED off
	if(fOnTime_inSecs == 0)
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);	//LED off
	}
	//Keep the LED on
	else if (fOffTime_inSecs == 0)
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	}
	//Blink LED
	else
	{
		//Set on time, off time and number of blinks
		g_OffTime = 10.0*fOffTime_inSecs;
		g_OnTime = 10.0*fOnTime_inSecs;
		g_NoOfBlinks = ucHowManyTimes;

		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//Begin with LED on

		//Start the timer
		LEDTimer_Start();
	}
}

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

	memset(pucFridgeCamID, '\0', FRIDGECAM_ID_SIZE);
	strcpy((char*)pucFridgeCamID, FRIDGECAM_NAME_PREFIX);

	pucTemp = pucFridgeCamID + strlen(FRIDGECAM_NAME_PREFIX);
	lRetVal = sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(uint8_t *)macAddressVal);

	for(i=0; i<SL_MAC_ADDR_LEN; i++)
	{
		//Left Nibble
		*pucTemp = (macAddressVal[i] & 0xF0) >> 4;	//4-nibble size.
		pucTemp++;

		//RightNibble
		*pucTemp = (macAddressVal[i] & 0x0F);
		pucTemp++;
	}

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

	UART_PRINT("FridgeCam ID: %s\n\r",pucFridgeCamID);
    return lRetVal;
}
