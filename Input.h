#ifndef Input_h
#define Input_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


class Input {
protected:
	int lastValue;

public:

	Listener* onValueChanged = NULL;

	uint8_t pin;
	bool internalPullUp = false;
	bool isAnalog = false;

	void setup() {
		pinMode(pin, INPUT);
		if (internalPullUp) {
			digitalWrite(pin, HIGH);
		}
		lastValue = -1;
	}

	void loop() {
		int value;
		if (isAnalog) {
			value = analogRead(pin);
		} else {
			value = digitalRead(pin);
		}
		if (value == lastValue) {
	    	return;
	    }

		if (onValueChanged != NULL) {
			ValueChangedEvent event;
			event.previousValue = lastValue;
			event.currentValue = value;
			onValueChanged->handleEvent(&event);
		}

		lastValue = value;

	}

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
		print->print(F("\""));
		print->print(instanceName);
		print->print(F("\":"));
		print->print(lastValue);
	}

};

#endif
