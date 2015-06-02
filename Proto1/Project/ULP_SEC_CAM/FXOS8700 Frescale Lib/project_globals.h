#include "build.h"

// project globals structure
struct ProjectGlobals
{
	// bluetooth flags
	uint8 AngularVelocityPacketOn;
	uint8 DebugPacketOn;
	uint8 RPCPacketOn;
	uint8 AltPacketOn;

	// bluetooth quaternion type
	quaternion_type QuaternionPacketType;

	// default quaternion type
	quaternion_type DefaultQuaternionPacketType;

	// magnetic packet variable identifier
	int16 MagneticPacketID;

	// packet number
	uint8 iPacketNumber;

	// MPL3115 found flag
	int8 iMPL3115Found;

	// global counter incrementing each iteration of sensor fusion (typically 25Hz)
	int32 loopcounter;
};

// globals defined in mqx_tasks.c
extern struct ProjectGlobals globals;
