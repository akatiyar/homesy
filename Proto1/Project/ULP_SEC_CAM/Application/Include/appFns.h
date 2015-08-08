/*
 * appFns.h
 *
 *  Created on: 30-Jun-2015
 *      Author: Chrysolin
 */

#ifndef APPFNS_H_
#define APPFNS_H_

extern int32_t User_Configure();
extern int32_t application_fn();

extern uint8_t Get_BatteryPercent();

extern int16_t IsLightOff(uint16_t usThresholdLux);
extern int32_t PowerDown_LightSensor();

extern float_t get_angle();
extern void check_doorpos();
extern float_t Calculate_DoorOpenThresholdAngle(float_t angle_40, float_t angle_90);
extern int16_t angleCheck_Initializations();
extern int16_t magnetometer_initialize();

extern int32_t NWP_SwitchOn();
extern int32_t NWP_SwitchOff();

extern int8_t IsInterruptFromLightSensor();
extern int8_t IsInterruptFromBatteryADC();
extern int32_t ClearInterrupt_IOExpander();

extern int16_t IsLongPress();

extern void Reset_byStarvingWDT();

#endif /* APPFNS_H_ */
