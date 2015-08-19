#ifndef __MT9D111_H__
#define __MT9D111_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

/*!
    \brief                      This function initilizes the camera sensor

    \param[in]                  None

    \return                     0 - Success
                               -1 - Error

    \note
    \warning
*/
long CameraSensorInit();
long CameraSensorInit_SettingsFromFlash();
long StartCapture_SettingsFromFlash();
long WriteConfigRegFile_toFlash();
/*!
    \brief                      Configures sensor in JPEG mode

    \param[in]                  None

    \return                     0 - Success
                               -1 - Error

    \note
    \warning
*/
#include <stdint.h>

long StartSensorInJpegMode();
long RestartSensorInJpegMode();

long RegStatusRead(uint16_t* pusRegVal);

long ResetImageSensorMCU();
long ReadMCUBootModeReg();

long AnalogGainReg_Read();
long PCLK_Rate_read();
int32_t ReadImageConfigReg(uint16_t *RegValues);
long DigitalGainRegs_Read();
long CCMRegs_Read();
long ShutterRegs_Read();

long ReadAllAEnAWBRegs();

long WriteAllAEnAWBRegs();

long SoftReset_ImageSensor();

long disableAWB();
long enableAWB();
long enableAE();
long disableAE();

long LL_Configs();

long Verify_ImageSensor();

long JPEGDataLength_read();

long Variable_Read(uint16_t usVariableName, uint16_t* pusRegVal);
long Variable_Write(uint16_t usVariableName, uint16_t usRegVal);
long Reg_Write(uint8_t RegPage, uint16_t usRegAddr, uint16_t usRegVal);
long Reg_Read(uint8_t RegPage, uint16_t usRegAddr, uint16_t* usRegVal);

int32_t SetShutterWidth(uint16_t ShutterWidth);
int32_t SetAnalogGain(uint8_t G1Gain,uint8_t BGain,uint8_t RGain,uint8_t G2Gain);
int32_t SetDigitalGain(uint8_t G1Gain,uint8_t BGain,uint8_t RGain,uint8_t G2Gain);
int32_t SetInitialGain(uint8_t ValChange, bool IsInc);
int32_t ReadGainReg(uint16_t* Gains);

int32_t EnterStandby_mt9d111(uint8_t ucMethod);
int32_t ExitStandby_mt9d111(uint8_t ucMethod);
#define SOFT_STANDBY				1
#define HARD_STANDBY				2

#define ANALOG_GAIN_BITS			0x0180
#define DIGITAL_GAIN_BITS			0x0600
#define INITIAL_GAIN_BITS			0x007F

int32_t Refresh_mt9d111Firmware();
int32_t BeginCapture_MT9D111();

int32_t Read_AllRegisters();
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif //__MT9D111_H__
