/*
 * wlan_event_handler.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <app_fns.h>
#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
    {
        //DEBG_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'-Applications
            // can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            RELEASE_PRINT("Connected to AP: %s\n", g_ucConnectionSSID);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_IP_AQUIRED);
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
            	RELEASE_PRINT("Disconnected from AP on app request\n",
                           g_ucConnectionSSID);
            }
            else
            {
                RELEASE_PRINT("Disconnected from AP on ERROR: %x\n",
                		pEventData->reason_code);

//				if(SL_INVALID_INFORMATION_ELEMENT == pEventData->reason_code)
//				{
//					DEBG_PRINT("Reason: SL_INVALID_INFORMATION_ELEMENT\n\r");
//				}
//				else if(SL_MESSAGE_INTEGRITY_CODE_MIC_FAILURE == pEventData->reason_code)
//				{
//					DEBG_PRINT("Reason: SL_MESSAGE_INTEGRITY_CODE_MIC_FAILURE\n\r");
//				}
//				else if(SL_FOUR_WAY_HANDSHAKE_TIMEOUT == pEventData->reason_code)
//				{
//					DEBG_PRINT("Reason: SL_FOUR_WAY_HANDSHAKE_TIMEOUT\n\r");
//				}
//              else if (SL_ROAMING_TRIGGER_BSS_LOSS == pEventData->reason_code)
//				{
//					DEBG_PRINT("Reason: SL_ROAMING_TRIGGER_BSS_LOSS\n\r");
//				}
                // cc3200 gets stuck after this. So, a workaround is to reset
                //and begin again
#ifdef USB_DEBUG
                while(1);
#else
                Reset_byStarvingWDT();
                //PRCMSOCReset();
#endif
                /*sl_Stop(0);
                DEBG_PRINT("NWP stop done");
                sl_Start(0,0,0);
                DEBG_PRINT("NWP restart done");*/
            }
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
        	RELEASE_PRINT("Mobile Station connected to CC3200-AP\n");
        	g_PhoneConnected_ToCC3200AP_flag = 1;
        }
        break;
        default:
        {
            DEBG_PRINT("Unexpected WLANEvent:0x%x\n", pWlanEvent->Event);
        }
        break;
    }
}

