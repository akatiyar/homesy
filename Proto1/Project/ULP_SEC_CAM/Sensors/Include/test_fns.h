/*
 * test_fns.h
 *
 *  Created on: 01-Apr-2015
 *      Author: Chrysolin
 */
#ifdef TEST_MODULES_INCLUDE
#ifndef TEST_FNS_H_
#define TEST_FNS_H_

void TestFxosApiReadWrite(void *pvParameters);
void TestSi7020ApiReadWrite(void *pvParameters);
void TestIsl29035Api(void *pvParameters);
void collectTxitSensorData(void *pvParameters);

#endif /* TEST_FNS_H_ */
#endif
