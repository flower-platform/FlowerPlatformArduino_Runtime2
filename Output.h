/*
 *  Author: Claudiu Matei
 */

#ifndef Output_h
#define Output_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

class Output {
protected:
	int lastValue;

public:
	uint8_t pin;

	Listener* valueChangedListener = NULL;

	bool isPwm = false;

	void setup() {
	    pinMode(pin, OUTPUT);
	    lastValue = -1;
	}

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
		print->print(F("\""));
		print->print(instanceName);
		print->print(F("\": "));
		print->print(lastValue);
	}

	void setHigh() {
//		if (lastValue) {
//			return;
//		}
		setValue(HIGH);
	}

	void setLow() {
//		if (!lastValue) {
//			return;
//		}
		setValue(LOW);
	}

	void setValue(int value) {
		if (isPwm) {
			analogWrite(pin, value);
		} else {
			digitalWrite(pin, value);
		}

		if (valueChangedListener != NULL) {
			ValueChangedEvent event;
			event.previousValue = lastValue;
			event.currentValue = value;
			valueChangedListener->handleEvent(&event);
		}

		lastValue = value;
	}

	void toggleHighLow() {
		if (lastValue) {
			setValue(LOW);
		} else {
			setValue(HIGH);
		}
	}

};


#endif
