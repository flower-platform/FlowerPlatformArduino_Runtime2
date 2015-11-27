/*
 *  Author: Claudiu Matei
 */

#ifndef MotorController_h
#define MotorController_h

#include <Arduino.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

class MotorController {

public:
	uint8_t enablePin;
	uint8_t controlPin1;
	uint8_t controlPin2;

	void setup() {
//	    pinMode(enablePin, OUTPUT);
	    pinMode(controlPin1, OUTPUT);
	    pinMode(controlPin2, OUTPUT);

//	    digitalWrite(enablePin, LOW);
	    digitalWrite(controlPin1, LOW);
	    digitalWrite(controlPin2, LOW);
	}

	void setSpeed(bool forward, int speed) {
		analogWrite(controlPin1, forward * speed);
		analogWrite(controlPin2, !forward * speed);
//		analogWrite(enablePin, speed);
	}

};


#endif
