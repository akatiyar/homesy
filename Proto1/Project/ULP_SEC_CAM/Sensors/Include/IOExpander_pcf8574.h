/*
 * IOExpander_pcf8574.h
 *
 *  Created on: 30-Jul-2015
 *      Author: Chrysolin
 */

#ifndef SENSORS_INCLUDE_IOEXPANDER_PCF8574_H_
#define SENSORS_INCLUDE_IOEXPANDER_PCF8574_H_

#define PCF8574_I2C_ADDRESS					0x20	//Considering A0, A1, A2 are tied to ground

#define SYS_OFF								0b00000001
#define LIGHTSENSOR_TRIGGER					0b00000010
#define CHG_SIGNAL							0b00000100
#define PWR_GOOD_SIGNAL						0b00001000
#define ACCEL_TRIGGER1						0b00010000
#define ACCEL_TRIGGER2						0b00100000
#define CONFIG_SW1							0b01000000
#define BATTERY_ADC_TRIGGER					0b10000000

int32_t ReadPorts_pcf8574(uint8_t* pucData);

#endif /* SENSORS_INCLUDE_IOEXPANDER_PCF8574_H_ */
