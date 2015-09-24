/*
 * netwpp_event_handler.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */


#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
    {
        LOOP_FOREVER();
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            DEBG_PRINT("IP Acquired: IP=%d.%d.%d.%d\n",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0));
        }
        break;
        case SL_NETAPP_IP_LEASED_EVENT:
        {
        	DEBG_PRINT("CC3200-AP has leased IP to Mobile Station\n");
        }
        break;
        default:
        {
            DEBG_PRINT("Unexpected NetAppEvent:0x%x\n", pNetAppEvent->Event);
        }
        break;
    }
}
