#ifndef ASGPROCESSOR_MKII_PROTOCOL_H
#define ASGPROCESSOR_MKII_PROTOCOL_H

#include <inttypes.h>

#ifdef _MSC_VER
#define PACKED_STRUCT struct
#pragma pack( push, 1 )
#else
#define PACKED_STRUCT struct __attribute__ ((packed))
#endif

namespace usbSensor_protocol {

	static const int packetSize = 16;

	typedef uint8_t Packet[ packetSize ];

	enum PacketType {
		pktIAmStillAlive,
		pktTrigger
	};

	// PACKETS

	PACKED_STRUCT Packet_IAmStillAlive{
		static const PacketType packetType = pktIAmStillAlive;
		bool isBarrierOk;
		bool isSignalQualityOk;
	};

	PACKED_STRUCT Packet_Trigger{
		static const PacketType packetType = pktTrigger;
	};

}

#ifdef _MSC_VER
#pragma pack( pop )
#endif

#endif // ASGPROCESSOR_MKII_PROTOCOL_H
