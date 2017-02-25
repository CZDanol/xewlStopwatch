#ifndef __USB_h__
#define __USB_h__

#include <util/delay.h>
#include "vusb_usbdrv.h"

class HIDUSB {
	friend usbMsgLen_t usbFunctionSetup( uchar data[ 8 ] );
	friend uint8_t usbFunctionWrite( uint8_t *data, uint8_t len );
	friend uint8_t usbFunctionRead( uint8_t *data, uint8_t len );

public:
	static const int packetSize = 16;
	typedef uint8_t Packet[ packetSize ];

public:
	inline HIDUSB() {
		received_ = false;
		toSend_ = false;
		isActive_ = false;
	}

	inline void begin() {
		cli();
		usbInit();
		usbDeviceDisconnect();
		uchar i = 0;
		while( --i ) {             // fake USB disconnect for > 250 ms
			_delay_ms( 10 );
		}

		usbDeviceConnect();

		sei();
	}
	inline bool active() {
		return isActive_;
	}

	inline void refresh() {
		usbPoll();
	}
	inline void delay( unsigned long ms ) {
		unsigned long startMls = millis();

		while( millis() < startMls + ms )
			refresh();
	}

	bool receive( Packet &target );
	void send( const Packet &source );

private:
	Packet receiveBuffer_, sendBuffer_;
	bool received_, toSend_;
	bool isActive_;

};

extern HIDUSB hidUSB;

#endif // __USB_h__
