#include <Arduino.h>

#include "USB.h"

bool HIDUSB::receive( Packet &target ) {
	if( !received_ )
		return false;

	memcpy( target, receiveBuffer_, sizeof( Packet ) );

	received_ = false;
	return true;
}
void HIDUSB::send( const Packet &source ) {
	// Wait till the previous packet is not sent (or timeout)
	if( toSend_ ) {
		unsigned long endMls = millis() + 5;
		while( toSend_ && millis() < endMls )
			refresh();
	}

	toSend_ = true;
	memcpy( sendBuffer_, source, sizeof( Packet ) );
}



/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

	PROGMEM const char usbHidReportDescriptor[ 22 ] = {    /* USB report descriptor */
			0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
			0x09, 0x01,                    // USAGE (Vendor Usage 1)
			0xa1, 0x01,                    // COLLECTION (Application)
			0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
			0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
			0x75, 0x08,                    //   REPORT_SIZE (8)
			0x95, 0x10,                    //   REPORT_COUNT (16)
			0x09, 0x00,                    //   USAGE (Undefined)
			0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
			0xc0                           // END_COLLECTION
	};
	/* Since we define only one feature report, we don't use report-IDs (which
	 * would be the first byte of the report). The entire report consists of 1
	 * opaque data bytes.
	 */

	 /* ------------------------------------------------------------------------- */

	static unsigned writeBytesRemaining = 0, readBytesRemaining = 0;
	static uint8_t *writeAddr = 0, *readAddr = 0;

	usbMsgLen_t usbFunctionSetup( uchar data[ 8 ] ) {
		hidUSB.isActive_ = true;

		usbRequest_t *rq = (usbRequest_t*) ( (void *) data );

		if( ( rq->bmRequestType & USBRQ_TYPE_MASK ) != USBRQ_TYPE_CLASS )
			return 0;

		if( rq->bRequest == USBRQ_HID_GET_REPORT && hidUSB.toSend_ ) {
			readBytesRemaining = sizeof( HIDUSB::Packet );
			readAddr = hidUSB.sendBuffer_;

			return USB_NO_MSG;
		}

		if( rq->bRequest == USBRQ_HID_SET_REPORT && !hidUSB.received_ ) {
			writeBytesRemaining = sizeof( HIDUSB::Packet );
			writeAddr = hidUSB.receiveBuffer_;

			return USB_NO_MSG;
		}
	}

	uint8_t usbFunctionWrite( uint8_t *data, uint8_t len ) {
		if( len > writeBytesRemaining )
			len = writeBytesRemaining;

		memcpy( writeAddr, data, len );

		writeAddr += len;
		writeBytesRemaining -= len;

		if( writeBytesRemaining == 0 && len )
			hidUSB.received_ = true;

		return writeBytesRemaining == 0;
	}

	uint8_t usbFunctionRead( uint8_t *data, uint8_t len ) {
		if( len > readBytesRemaining )
			len = readBytesRemaining;

		memcpy( data, readAddr, len );
		readAddr += len;
		readBytesRemaining -= len;

		if( readBytesRemaining == 0 && len )
			hidUSB.toSend_ = false;

		return len;
	}

#ifdef __cplusplus
} // extern "C"
#endif


HIDUSB hidUSB;
