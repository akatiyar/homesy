/*
 * http_server_callback.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include <nwp.h>
#include "app_common.h"
#include "network_related_fns.h"
#include "mt9d111.h"

char Token[100]="";
char Value[100]="";

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives GET and POST HTTP
//!	Tokens (particularly from the Androind app).
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent,
        SlHttpServerResponse_t *pSlHttpServerResponse)
{
	uint8_t G1Gain,RGain,BGain,G2Gain;
	uint8_t GainVal=0;
	uint16_t ReadGain[4];

    switch (pSlHttpServerEvent->Event)
    {
        case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
        {
        	uint16_t Token_Len = pSlHttpServerEvent->EventData.httpPostData.token_name.len;
        	uint16_t Value_Len =  pSlHttpServerEvent->EventData.httpPostData.token_value.len;
        	memcpy(Value,  pSlHttpServerEvent->EventData.httpPostData.token_value.data,Value_Len);
        	memcpy(Token,  pSlHttpServerEvent->EventData.httpPostData.token_name.data,Token_Len);
        	Token[Token_Len] = '\0';
        	Value[Value_Len] = '\0';
        	DEBG_PRINT("POST Token: %s Value : %s\n",Token, Value);

        	//angle90 , angle40
        	if(0 == memcmp(Token, TOK_ANGLE, Token_Len))
			{
        		if(0 == memcmp(Value, "Angle90", Value_Len))
        		{
        			g_ucAngle90 = BUTTON_PRESSED;

        		}
        		else if(0 == memcmp(Value, "Angle40", Value_Len))
        		{
        			g_ucAngle40 = BUTTON_PRESSED;
        		}
			}
           	else if(0 == memcmp(Token, TOK_CALIB, Token_Len))
			{
				if(0 == memcmp(Value, "Calibrate", Value_Len))
				{
					g_ucCalibration = BUTTON_PRESSED;
				}
			}
        	else if(0 == memcmp(Token, TOK_SSID, Token_Len))		//__SL_P_USD
        	{
        		memcpy(g_cWlanSSID,  Value, Value_Len);
        		g_cWlanSSID[pSlHttpServerEvent->EventData.httpPostData.token_value.len] = 0;
        	}
        	//Security Type
        	else if(0 == memcmp(Token, TOK_KEYTYPE, Token_Len))		//__SL_P_USE
        	{
				if(Value[0] == '0')
				{
					g_SecParams.Type =  SL_SEC_TYPE_OPEN;//SL_SEC_TYPE_OPEN
				}
				else if(Value[0] == '1')
				{
					g_SecParams.Type =  SL_SEC_TYPE_WEP;//SL_SEC_TYPE_WEP
				}
				else if(Value[0]== '2')
				{
					g_SecParams.Type =  SL_SEC_TYPE_WPA;//SL_SEC_TYPE_WPA
				}
				else
				{
					g_SecParams.Type =  SL_SEC_TYPE_OPEN;//SL_SEC_TYPE_OPEN
				}
				g_cWlanSecurityType[0] = g_SecParams.Type;
				g_cWlanSecurityType[1] = 0;
        	}

        	//Security Key
        	else if(0 == memcmp(Token, TOK_KEY, Token_Len))		//__SL_P_USF
        	{
				memcpy(g_cWlanSecurityKey,Value, Value_Len);
				g_cWlanSecurityKey[Value_Len] = 0;
				g_SecParams.Key = g_cWlanSecurityKey;
				g_SecParams.KeyLen = Value_Len;
			}
        	else if(0 == memcmp(Token, TOK_CONFIG_WIFI, Token_Len))		//__SL_P_US0
        	{
        		g_ucConfig = BUTTON_PRESSED;
            }
        	else if(0 == memcmp(Token, TOK_EXIT, Token_Len))
        	{
        		if(0 == memcmp(Value, "Exit", Value_Len))
				{
        			g_ucExitButton = BUTTON_PRESSED;
				}
        	}
        	else if(0 == memcmp(Token, TOK_ACTION1, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value,"Integ",5))
        		{
        			//Copy shutter Width
        			uint16_t ShutterWidth = atoi(&Value[6]);
        			SetShutterWidth(ShutterWidth);
        		}

            }
        	else if(0 == memcmp(Token, TOK_ACTION2, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value,"AGain",5))
        		{
        			//Copy Analog Gain
        			G1Gain = atoi(&Value[6]);
        			RGain = atoi(&Value[9]);
        			BGain = atoi(&Value[12]);
        			G2Gain = atoi(&Value[15]);
        			SetAnalogGain(G1Gain,BGain,RGain,G2Gain);
        		}
        		if(0 == memcmp(Value,"DGain",5))
        		{
        			//Copy Digital Gain
					G1Gain = atoi(&Value[6]);
					RGain = atoi(&Value[9]);
					BGain = atoi(&Value[12]);
					G2Gain = atoi(&Value[15]);
					SetDigitalGain(G1Gain,BGain,RGain,G2Gain);
        		}
        		if(0 == memcmp(Value,"IGain",5))
        		{
        			GainVal = atoi(&Value[10]);

        			if(0 == memcmp(&Value[5],",Inc,",3))
        			{
						//Copy Initial Gain
        				SetInitialGain(GainVal,true);
        			}
        			else if(0 == memcmp(&Value[5],",Dec,",3))
        			{
						//Copy Initial Gain
        				SetInitialGain(GainVal,false);
        			}
					ReadGainReg(ReadGain);
        		}

            }
        	else if(0 == memcmp(Token, TOK_ACTION3, Token_Len))		//__SL_P_US0
        	{

        		if(0 == memcmp(Value, "Restart", Value_Len))
				{
        			g_ucActionButton = BUTTON_PRESSED;
					g_ucAction = CAM_RESTART_CAPTURE;
				}
            }
        	else if(0 == memcmp(Token, TOK_ACTION4, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "OTA", Value_Len))
				{
					g_ucActionButton = BUTTON_PRESSED;
					g_ucAction = OTA_FIRMWARE_UPDATE;
				}
            }
        	else if(0 == memcmp(Token, TOK_PREVIEW, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "Start", Value_Len))
				{
        			g_ucPreviewStart = BUTTON_PRESSED;
				}
        		else if(0 == memcmp(Value, "Stop", Value_Len))
				{
        			g_ucPreviewStop = BUTTON_PRESSED;
				}
            }
        	else if(0 == memcmp(Token, TOK_AWB, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "AWBON", Value_Len))
				{
        			g_ucAWBOn = BUTTON_PRESSED;
				}
        		else if(0 == memcmp(Value, "AWBOFF", Value_Len))
				{
        			g_ucAWBOff = BUTTON_PRESSED;
				}
            }
        	else if(0 == memcmp(Token, TOK_SAVE, Token_Len))		//__SL_P_US0
        	{
        		if(0 == memcmp(Value, "SAVE", Value_Len))
				{
        			g_ucSAVE = BUTTON_PRESSED;
				}
            }

        }
        break;

      default:
          break;
    }
}
