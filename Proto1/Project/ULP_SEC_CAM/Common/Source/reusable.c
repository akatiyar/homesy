#include "app_common.h"
#include "string.h"

const char pcDigits[] = "0123456789"; /* variable used by itoa function */

//*****************************************************************************
//	intToASCII() converts an integer (64-bit - long long) into ASCII string
//	This function has been adapted from itoa() in CC3200 SDK network_if.c
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters including the null character at the
//!					end of the output string
//
//*****************************************************************************
unsigned short intToASCII(long long cNum, char *cString)
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

//		start_100mSecTimer();

//		ulTimeDuration_ms = get_timeDuration();
//		stop_100mSecTimer();
//		DEBG_PRINT("Read SensData - %d ms\n\r", ulTimeDuration_ms);
