/*
 * socket_event_handler.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
    {
        //DEBG_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status )
            {
                case SL_ECLOSE:
                    DEBG_PRINT("Close socket (%d) operation "
                    "failed to transmit all queued packets\n",
					pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default:
                    DEBG_PRINT("TX FAILED reason: (%d)\n",
                    		pSock->socketAsyncEvent.SockTxFailData.status);
//                    if(SL_ENOTCONN == pSock->socketAsyncEvent.SockTxFailData.status)
//                    {
//                    	DEBG_PRINT("-107 = SL_ENOTCONN = "
//                    			"Transport endpoint is not connected\n\r");
//                    }
            }
            break;

        default:
            DEBG_PRINT("Unexpected SockEvent: 0x%x\n",pSock->Event);
    }

}
