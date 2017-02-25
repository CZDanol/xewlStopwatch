#include "USB.h"
#include "usbProtocol.h"

enum Pin {
	pinLed
};


using namespace usbSensor_protocol;

template< typename T >
void sendPacket( const T &packet ) {
	Packet pkt;
	pkt[ 0 ] = T::packetType;
	memcpy( pkt + 1, &packet, sizeof( T ) );
	hidUSB.send( pkt );
}

void setup() {
	pinMode( pinLed, OUTPUT );

	hidUSB.begin();
}

const unsigned long delayTime = 256;
const double longTimeAvgCoef = 0.03;

unsigned long lastTrigger = -2000000;

void loop() {
	unsigned long us = micros();
	// Ingore first second after startup
	static unsigned long lastSignal = 0;

	// Analyze and process input
	{
		static double longTimeAvg = 0;

		const uint16_t data = analogRead( 1 );
		longTimeAvg = data * longTimeAvgCoef + longTimeAvg * ( 1.0 - longTimeAvgCoef );

		static bool raising = false;
		static unsigned long start = 0;

		// Measure frequency of modulation
		if( !raising && data < longTimeAvg * 0.985 ) {
			// If it matches 1ms, consider it ours
			if( abs( ( us - start ) - 1024 ) < 256 )
				lastSignal = us;

			raising = true;
			start = us;
		} else if( raising && data > longTimeAvg * 1.015 ) {
			raising = false;
		}
	}

	// Debounce and process the signal
	{
		bool hasSignal = ( us - lastSignal ) < 8000;
		static bool prevHasSignal = false;
		static unsigned long signalDebounceUntil = 2000000;

		if( hasSignal != prevHasSignal && us > signalDebounceUntil ) {
			if( !hasSignal ) {
				signalDebounceUntil = us + 1000000;
				lastTrigger = us;

				Packet_Trigger pkt;
				sendPacket( pkt );

			} else if( us > signalDebounceUntil )
				signalDebounceUntil = us + 4096;
		}

		prevHasSignal = hasSignal;
	}

	// Update led
	{
		if( us - lastTrigger < 1000000 )
			digitalWrite( pinLed, HIGH );
		else if( us - lastSignal > 5000000 )
			digitalWrite( pinLed, us / 1024 / 128 % 2 );
		else
			digitalWrite( pinLed, us / 1024 / 64 % 8 == 0 );
	}

	// USB stuff
	static unsigned long nextHidHandling = 0;
	if( us >= nextHidHandling ) {
		nextHidHandling = us + 2000;
		hidUSB.refresh();

		static unsigned long nextAlive = 0;

		if( nextAlive <= us ) {
			nextAlive = us + 500000;
			sendPacket( Packet_IAmStillAlive{ us - lastSignal <= 5000000 } );
		}
	}

	const unsigned long sleepTime = delayTime - ( micros() - us );
	if( sleepTime < delayTime )
		delayMicroseconds( sleepTime );
}