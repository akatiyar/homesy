/*
 * fns_to_be_called_from_mainTask.c
 *
 *  Created on: 05-Jun-2015
 *      Author: Chrysolin
 */




void fxos_testing()
{
	float_t fYaw_closedDoor;

	fxos_calibrate();
	fxos_get_initialYaw(&fYaw_closedDoor);

	//fxos_calibrate();
	fxos_waitFor40Degrees(fYaw_closedDoor);

	while(1);
}

int32_t initial_magnetometerReadings()
{
	float_t fYaw_closedDoor;
	int32_t lRetVal;

	fxos_get_initialYaw(&fYaw_closedDoor);
	DEBG_PRINT("Closed Door: %3.2f\n\r", fYaw_closedDoor);
	//
	// Save initial magnetometer readings in flash
	//
	//lRetVal = sl_Start(0, 0, 0);
	lRetVal = NWP_SwitchOn();
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = CreateFile_Flash((uint8_t*)MAGN_INIT_VALS_FILE_NAME,
								MAGN_INIT_VALS_FILE_MAXSIZE);
	int32_t lFileHandle;
	lRetVal = WriteFile_ToFlash((uint8_t*)&fYaw_closedDoor,
								(uint8_t*)MAGN_INIT_VALS_FILE_NAME, 4, 0,
								SINGLE_WRITE, &lFileHandle);

	lRetVal = sl_Stop(0xFFFF);
	//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

int32_t waitfor40degrees_fxos()
{
	float_t fYaw_closedDoor;
	int32_t lRetVal;

	//
	// Read initial magnetometer readings from flash
	//
	//lRetVal = sl_Start(0, 0, 0);
	lRetVal = NWP_SwitchOn();
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = ReadFile_FromFlash((uint8_t*)&fYaw_closedDoor,
									(uint8_t*)MAGN_INIT_VALS_FILE_NAME, 3, 0 );

	lRetVal = sl_Stop(0xFFFF);
	//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	//fxos_calibrate();
	//DEBG_PRINT("\n\rc** Calibration over\n\r");
	fxos_waitFor40Degrees(fYaw_closedDoor);

	return 1;
}

void fxos_testing_2()
{
	//fxos_calibrate();
	//LED_Blink(30, 1);
	fxos_calibrate();


	DEBG_PRINT("\n\ra** Initial Door Closed Pos\n\r");
	initial_magnetometerReadings();
	DEBG_PRINT("\n\rb**Going to Calibrate\n\r");
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	waitfor40degrees_fxos();
	LED_Blink(10, .3);
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	UtilsDelay(5*80000000/6);
	DEBG_PRINT("\n\rf**\n\r");
	//while(1);
}

void fxos_testing_3()
{
	LED_Blink(30, 1);

	DEBG_PRINT("\n\ra** Initial Door Closed Pos\n\r");
	initial_magnetometerReadings();
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	waitfor40degrees_fxos();
	LED_Blink(10, .3);
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	UtilsDelay(5*80000000/6);
	DEBG_PRINT("\n\rf**\n\r");
	//while(1);
}

void fxos_testing_4()
{
	//LED_Blink(30, 1);
	while(1)
	{
		DEBG_PRINT("\n\ra** Initial Door Closed Pos\n\r");

		float_t fYaw_closedDoor;
		fxos_get_initialYaw(&fYaw_closedDoor);
		DEBG_PRINT("Closed Door: %3.2f\n\r", fYaw_closedDoor);

		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
		fxos_waitFor40Degrees(fYaw_closedDoor);
		LED_Blink(10, .3);
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
		UtilsDelay(5*80000000/6);
		DEBG_PRINT("\n\rf**\n\r");
	}
	//while(1);
}

void Testing_fxos()
{
	verifyAccelMagnSensor();
	//fxos_calibrate();
	fxos_main();
	//fxos_testing();

	//	while(1)
	//	{
	//		fxos_testing_2();
	//	}

	//fxos_testing_3();
}
