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

extern float_t get_angle();
extern void check_doorpos();
extern int16_t angleCheck_Initializations();

#endif /* APPFNS_H_ */
