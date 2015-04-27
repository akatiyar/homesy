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

/*!
    \brief                      Configures sensor in JPEG mode

    \param[in]                  None

    \return                     0 - Success
                               -1 - Error

    \note
    \warning
*/
long StartSensorInJpegMode();

long RegStatusRead();

long AnalogGainReg_Read();
long DigitalGainRegs_Read();
long CCMRegs_Read();
long ShutterRegs_Read();

long ReadAllAEnAWBRegs();

long WriteAllAEnAWBRegs();

long disableAWB();
long enableAWB();
long enableAE();
long disableAE();
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif //__MT9D111_H__
