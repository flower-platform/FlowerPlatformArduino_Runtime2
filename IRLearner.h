/*
 *  Author: Claudiu Matei
 */

#ifndef IRLearner_h
#define IRLearner_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdbool.h>
#include <stdint.h>
#include <SdFat.h>

#define RESOLUTION 80
#define MAX_TIME_SPAN 10000
#define TIMEOUT 50000

/**
 * Uses PWM pin 2 to transmit
 */
class IRLearner {
protected:
	SdFat SD;

public:
	const char* name;
	uint8_t pin; // must be within [0..7]

	void setup() {
		SD.begin(4);
	}

	bool capture(const char* name) {
		uint16_t t;
	    uint16_t command[200];  // pulse time spans (must be multiplied by RESOLUTION to get real time in microseconds)
	    int n = 0; // number of pulses

	    // capture
	    bool capturing = true;
	    while (capturing) {
	    	t = 0;
	    	while (PIND & _BV(pin)) { // pin is HIGH (IR inactive)
	    		t++;
				delayMicroseconds(RESOLUTION);
				if (t > TIMEOUT) {
					return false;
				}
				else if (t >= MAX_TIME_SPAN && (n != 0)) {
					capturing = false;
					break;
				}
			}
			if (!capturing) {
				break;
			}
	    	command[n++] = t;

			t = 0;
			while (! (PIND & _BV(pin))) { // pin is LOW (IR active)
				t++;
				delayMicroseconds(RESOLUTION);
			}
			command[n++] = t;
	    }

		// Write command to SD card
		SD.chdir(true); // set current dir to root
	    SD.mkdir("ir-commands", false);
	    SD.chdir("ir-commands", true);
	    SD.remove(name);
		File f = SD.open(name, FILE_WRITE);
		f.println(n-1);
		for (int i = 1; i < n; i++) {
			f.println(command[i] * RESOLUTION);
		}
		f.close();
		return true;
	}

};

#endif
