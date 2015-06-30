////////////////////////////////////////////////////////////////////////////////
 /*	batteryVoltLvlSens_adc081c021.h
 *	Company Name : Soliton Technologies
 *	Description  : ADC chip APIs for battery monitoring
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////


#ifndef BATTERYVOLTLVLSENS_ADC081C021_H_
#define BATTERYVOLTLVLSENS_ADC081C021_H_



#include "app.h"
#include "math.h"

#define ADC_RESOLUTION							(256.0F)

int32_t PutInAlertMode_ADC081C021(float_t fAlertVoltageLimit);

int32_t Get_BatteryVoltageLevel_ADC081C021(uint8_t* pucADCValue);





#endif /* BATTERYVOLTLVLSENS_ADC081C021_H_ */
