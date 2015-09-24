/*
 * SNTP.c
 *
 *  Created on: 18-Sep-2015
 *      Author:
 */

#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"

//!    ######################### list of SNTP servers ##################################
//!    ##
//!    ##          hostname         |        IP       |       location
//!    ## -----------------------------------------------------------------------------
//!    ##   nist1-nj2.ustiming.org  | 165.193.126.229 |  Weehawken, NJ
//!    ##   nist1-pa.ustiming.org   | 206.246.122.250 |  Hatfield, PA
//!    ##   time-a.nist.gov         | 129.6.15.28     |  NIST, Gaithersburg, Maryland
//!    ##   time-b.nist.gov         | 129.6.15.29     |  NIST, Gaithersburg, Maryland
//!    ##   time-c.nist.gov         | 129.6.15.30     |  NIST, Gaithersburg, Maryland
//!    ##   ntp-nist.ldsbc.edu      | 198.60.73.8     |  LDSBC, Salt Lake City, Utah
//!    ##   nist1-macon.macon.ga.us | 98.175.203.200  |  Macon, Georgia
//!
//!    ##   For more SNTP server link visit 'http://tf.nist.gov/tf-cgi/servers.cgi'
//!    ###################################################################################
const char g_acSNTPserver[30] = "nist1-macon.macon.ga.us"; //Add any one of the above servers

// Tuesday is the 1st day in 2013 - the relative year
const char g_acDaysOfWeek2013[7][3] = {{"Tue"},
                                    {"Wed"},
                                    {"Thu"},
                                    {"Fri"},
                                    {"Sat"},
                                    {"Sun"},
                                    {"Mon"}};

const char g_acMonthOfYear[12][3] = {{"Jan"},
                                  {"Feb"},
                                  {"Mar"},
                                  {"Apr"},
                                  {"May"},
                                  {"Jun"},
                                  {"Jul"},
                                  {"Aug"},
                                  {"Sep"},
                                  {"Oct"},
                                  {"Nov"},
                                  {"Dec"}};

const char g_acNumOfDaysPerMonth[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};

const char g_acDigits[] = "0123456789";

struct
{
    unsigned long ulDestinationIP;
    int iSockID;
    unsigned long ulElapsedSec;
    short isGeneralVar;
    unsigned long ulGeneralVar;
    unsigned long ulGeneralVar1;
    char acTimeStore[30];
    char *pcCCPtr;
    unsigned short uisCCLen;
}g_TimeData;


long GetSNTPTime(SlDateTime_t *dateTime, unsigned char ucGmtDiffHr, unsigned char ucGmtDiffMins)
{

	SlSockAddr_t sAddr;
	SlSockAddrIn_t sLocalAddr;

/*
                            NTP Packet Header:

       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          Root  Delay                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root  Dispersion                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Reference Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Reference Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Originate Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Receive Timestamp (64)                     |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Transmit Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                 Key Identifier (optional) (32)                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                                                                |
      |                 Message Digest (optional) (128)                |
      |                                                                |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/
    char cDataBuf[48];
    long lRetVal = 0;
    int iAddrSize;
    //
    // Send a query ? to the NTP server to get the NTP time
    //
    memset(cDataBuf, 0, sizeof(cDataBuf));
    cDataBuf[0] = '\x1b';

    sAddr.sa_family = AF_INET;
    // the source port
    sAddr.sa_data[0] = 0x00;
    sAddr.sa_data[1] = 0x7B;    // UDP port number for NTP is 123
    sAddr.sa_data[2] = (char)((g_TimeData.ulDestinationIP>>24)&0xff);
    sAddr.sa_data[3] = (char)((g_TimeData.ulDestinationIP>>16)&0xff);
    sAddr.sa_data[4] = (char)((g_TimeData.ulDestinationIP>>8)&0xff);
    sAddr.sa_data[5] = (char)(g_TimeData.ulDestinationIP&0xff);

    lRetVal = sl_SendTo(g_TimeData.iSockID,
                     cDataBuf,
                     sizeof(cDataBuf), 0,
                     &sAddr, sizeof(sAddr));
    if (lRetVal != sizeof(cDataBuf))
    {
        // could not send SNTP request
        RETURN_ON_ERROR(SNTP_ERROR);
    }

    //
    // Wait to receive the NTP time from the server
    //
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = 0;
    sLocalAddr.sin_addr.s_addr = 0;
    if(g_TimeData.ulElapsedSec == 0)
    {
        lRetVal = sl_Bind(g_TimeData.iSockID,
                (SlSockAddr_t *)&sLocalAddr,
                sizeof(SlSockAddrIn_t));
    }

    iAddrSize = sizeof(SlSockAddrIn_t);

    lRetVal = sl_RecvFrom(g_TimeData.iSockID,
                       cDataBuf, sizeof(cDataBuf), 0,
                       (SlSockAddr_t *)&sLocalAddr,
                       (SlSocklen_t*)&iAddrSize);
    RETURN_ON_ERROR(lRetVal);

    //
    // Confirm that the MODE is 4 --> server
    //
    if ((cDataBuf[0] & 0x7) != 4)    // expect only server response
    {
         RETURN_ON_ERROR(SNTP_ERROR);  // MODE is not server, abort
    }
    else
    {
        unsigned char iIndex;

        //
        // Getting the data from the Transmit Timestamp (seconds) field
        // This is the time at which the reply departed the
        // server for the client
        //
        g_TimeData.ulElapsedSec = cDataBuf[40];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[41];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[42];
        g_TimeData.ulElapsedSec <<= 8;
        g_TimeData.ulElapsedSec += cDataBuf[43];

        //
        // seconds are relative to 0h on 1 January 1900
        //
        g_TimeData.ulElapsedSec -= TIME2013;

        //
        // in order to correct the timezone
        //
        g_TimeData.ulElapsedSec += (ucGmtDiffHr * SEC_IN_HOUR);
        g_TimeData.ulElapsedSec += (ucGmtDiffMins * SEC_IN_MIN);

        g_TimeData.pcCCPtr = &g_TimeData.acTimeStore[0];

        //
        // day, number of days since beginning of 2013
        //
        g_TimeData.isGeneralVar = g_TimeData.ulElapsedSec/SEC_IN_DAY;
        memcpy(g_TimeData.pcCCPtr,
               g_acDaysOfWeek2013[g_TimeData.isGeneralVar%7], 3);
        g_TimeData.pcCCPtr += 3;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // month
        //
        g_TimeData.isGeneralVar %= 365;
        for (iIndex = 0; iIndex < 12; iIndex++)
        {
        	g_TimeData.isGeneralVar -= g_acNumOfDaysPerMonth[iIndex];
            if (g_TimeData.isGeneralVar < 0)
                    break;
        }
        if(iIndex == 12)
        {
            iIndex = 0;
        }

        dateTime->sl_tm_mon = iIndex + 1;

        memcpy(g_TimeData.pcCCPtr, g_acMonthOfYear[iIndex], 3);
        g_TimeData.pcCCPtr += 3;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // date
        // restore the day in current month
        //
        g_TimeData.isGeneralVar += g_acNumOfDaysPerMonth[iIndex];
        g_TimeData.uisCCLen = intToASCII(g_TimeData.isGeneralVar + 1,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = '\x20';

        dateTime->sl_tm_day = g_TimeData.isGeneralVar + 1;
        //
        // time
        //
        g_TimeData.ulGeneralVar = g_TimeData.ulElapsedSec%SEC_IN_DAY;

        // number of seconds per hour
        g_TimeData.ulGeneralVar1 = g_TimeData.ulGeneralVar%SEC_IN_HOUR;

        // number of hours
        g_TimeData.ulGeneralVar /= SEC_IN_HOUR;

        dateTime->sl_tm_hour = g_TimeData.ulGeneralVar;

        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = ':';

        // number of minutes per hour
        g_TimeData.ulGeneralVar = g_TimeData.ulGeneralVar1/SEC_IN_MIN;

        // number of seconds per minute
        g_TimeData.ulGeneralVar1 %= SEC_IN_MIN;
        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = ':';
        g_TimeData.uisCCLen = intToASCII(g_TimeData.ulGeneralVar1,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;
        *g_TimeData.pcCCPtr++ = '\x20';

        //
        // year
        // number of days since beginning of 2013
        //
        g_TimeData.ulGeneralVar = g_TimeData.ulElapsedSec/SEC_IN_DAY;
        g_TimeData.ulGeneralVar /= 365;
        g_TimeData.uisCCLen = intToASCII(YEAR2013 + g_TimeData.ulGeneralVar,
        		g_TimeData.pcCCPtr);
        g_TimeData.pcCCPtr += g_TimeData.uisCCLen;

        dateTime->sl_tm_year = YEAR2013 + g_TimeData.ulGeneralVar;

        *g_TimeData.pcCCPtr++ = '\0';

        //DEBG_PRINT("Server: ");
        //DEBG_PRINT((char *)g_acSNTPserver);
        //DEBG_PRINT("\n");
        //DEBG_PRINT("NTP Time: ");
        //DEBG_PRINT(g_TimeData.acTimeStore);
        //DEBG_PRINT("\n");
    }
    return SUCCESS;
}

long Network_IF_GetHostIP( char* pcHostName,unsigned long * pDestinationIP )
{
	long lStatus=-1;

	lStatus = sl_NetAppDnsGetHostByName((signed char *) pcHostName,
                                            strlen(pcHostName),
                                            pDestinationIP, SL_AF_INET);
    RETURN_ON_ERROR(lStatus);

//    DEBG_PRINT("Get Host IP succeeded.\n\rHost: %s IP: %d.%d.%d.%d \n\r\n\r",
//                    pcHostName, SL_IPV4_BYTE(*pDestinationIP,3),
//                    SL_IPV4_BYTE(*pDestinationIP,2),
//                    SL_IPV4_BYTE(*pDestinationIP,1),
//                    SL_IPV4_BYTE(*pDestinationIP,0));

    return lStatus;
}

int32_t GetTimeNTP(SlDateTime_t *dateTime)
{
    int iSocketDesc;
    long lRetVal = -1;
    int count=0;

    //
    // Create UDP socket
    //
    iSocketDesc = sl_Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(iSocketDesc < 0)
    {
        ERR_PRINT(iSocketDesc);
    }

    //
    // Get the NTP server host IP address using the DNS lookup
    //
    lRetVal = Network_IF_GetHostIP((char*)g_acSNTPserver, \
                                    &g_TimeData.ulDestinationIP);
    if( lRetVal >= 0)
    {

        struct SlTimeval_t timeVal;
        timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
        timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
        lRetVal = sl_SetSockOpt(g_TimeData.iSockID,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
                        (unsigned char*)&timeVal, sizeof(timeVal));

        RETURN_ON_ERROR(lRetVal);

        do
        {
            //
            // Get the NTP time and display the time
            //
            lRetVal = GetSNTPTime(dateTime,GMT_DIFF_TIME_HRS, GMT_DIFF_TIME_MINS);
            if(lRetVal == 0)
            {
                //DEBG_PRINT("Time received\n");
                break;
            }

            //
            // Wait a while before resuming
            //
            MAP_UtilsDelay(SLEEP_TIME);
            count++;
        }
        while(count < 5);
        RETURN_ON_ERROR(lRetVal);
    }
    else
    {
        DEBG_PRINT("DNS lookup failed\n");
    }

    // Close the socket
    close(iSocketDesc);

    return SUCCESS;
}
