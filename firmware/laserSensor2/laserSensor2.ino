#include "USB.h"
#include "usbProtocol.h"

//#define TESTING_ENV

enum Pin {
	pinLed
};

const unsigned long period = 512;
const unsigned long delayTime = 128;

const int64_t convolutionThreshold = 6000;
const int64_t convolutionOkThreshold = 20000;

const unsigned long signalInterruptTimeout = 3000000;
const unsigned long signalTriggerTimeout = 500000;

using namespace usbSensor_protocol;

template< typename T >
void sendPacket( const T &packet ) {
	Packet pkt;
	pkt[ 0 ] = T::packetType;
	memcpy( pkt + 1, &packet, sizeof( T ) );
	hidUSB.send( pkt );
}

inline int8_t timeFunction( unsigned long t ) {
	return ( t / ( period / 2 ) ) % 2 * 2 - 1;
}

void setup() {
#ifdef TESTING_ENV
	Serial.begin( 9600 );
	Serial.println( "start" );
#else
	pinMode( pinLed, OUTPUT );

	hidUSB.begin();
#endif
}

void loop() {
	//return;
	unsigned long us = micros();
	static unsigned long idealUs = 0;

	// Ingore first second after startup
	static unsigned long lastTriggerTime = -100000000;
	static unsigned long lastSignalTime = -100000000;

	static int64_t lastConvolution = 0;
	static int64_t convolutionAvg = 0;
	static int64_t minConvo = convolutionOkThreshold;

	// Analyze and process input
	{
		static uint16_t convolutionSteps = 0;

		static int64_t convolution1 = 0;
		static int64_t convolution2 = 0;

		const int32_t data = analogRead( 1 );

		convolution1 += data * timeFunction( idealUs );
		convolution2 += data * timeFunction( idealUs + period / 4 );

		if( ++convolutionSteps == 8 ) {
			lastConvolution = convolution1 * convolution1 + convolution2 * convolution2;
			convolutionAvg += lastConvolution - convolutionAvg / 2;

			if( convolutionAvg < minConvo )
				minConvo = convolutionAvg;

#ifdef TESTING_ENV
			Serial.println( int( lastConvolution ) );
			Serial.println();
#endif

			// Debounce and process the signal
			{
				bool hasSignal = convolutionAvg > convolutionThreshold;
				static bool prevHasSignal = false;

				static unsigned long signalDebounceUntil = 2000000;

				if( hasSignal )
					lastSignalTime = us;

				if( hasSignal != prevHasSignal && us > signalDebounceUntil ) {
					if( !hasSignal ) {
						signalDebounceUntil = us + 1000000;
						lastTriggerTime = us;

#ifndef TESTING_ENV
						Packet_Trigger pkt;
						sendPacket( pkt );
#endif

					} else if( us > signalDebounceUntil )
						signalDebounceUntil = us + 4096;
				}

				prevHasSignal = hasSignal;
			}

			convolutionSteps = 0;
			convolution1 = 0;
			convolution2 = 0;
		}
	}

#ifndef TESTING_ENV
	// USB stuff
	static unsigned long nextPeripheralHandling = 0;
	if( us >= nextPeripheralHandling ) {
		nextPeripheralHandling = us + 10000;
		hidUSB.refresh();

		// Update led
		{
			if( us - lastTriggerTime < signalTriggerTimeout )
				digitalWrite( pinLed, HIGH );
			else if( us - lastSignalTime > signalInterruptTimeout )
				digitalWrite( pinLed, us / 1024 / 128 % 2 );
			else
				digitalWrite( pinLed, us / 1024 / 64 % 8 == 0 );
		}

		static unsigned long nextAlive = 0;

		if( nextAlive <= us ) {
			nextAlive = us + 500000;
			sendPacket( Packet_IAmStillAlive{ us - lastSignalTime <= signalInterruptTimeout, minConvo >= convolutionOkThreshold } );

			minConvo = convolutionOkThreshold;
		}
	}
#endif

	idealUs += delayTime;

	const unsigned long thisLoopTook = micros() - us;
	if( thisLoopTook < delayTime )
		delayMicroseconds( delayTime - thisLoopTook );
}