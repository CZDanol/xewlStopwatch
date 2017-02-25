//#define LAYOUT_1
#define LAYOUT_2

#ifdef LAYOUT_1

enum {
	pinLaser,
	pinLed
};

#elif defined(LAYOUT_2)

#define HAS_PIN_GROUND

enum {
	pinGnd,
	pinLed,
	pinLaser
};

#endif

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode( pinLed, OUTPUT );
	pinMode( pinLaser, OUTPUT );

	digitalWrite( pinLed, HIGH );
	digitalWrite( pinLaser, HIGH );

#ifdef HAS_PIN_GROUND
	pinMode( pinGnd, OUTPUT );
	digitalWrite( pinGnd, LOW );
#endif
}

const unsigned long period = 512;
inline int8_t timeFunction( unsigned long t ) {
	return ( t / ( period / 2 ) ) % 2 * 2 - 1;
}

// the loop function runs over and over again until power down or reset
void loop() {
	digitalWrite( pinLaser, timeFunction( micros() ) > 0 );
	//digitalWrite( pinLed, timeFunction( micros() ) > 0 );
	digitalWrite( pinLed, millis() / 64 % 8 == 0 );
}
