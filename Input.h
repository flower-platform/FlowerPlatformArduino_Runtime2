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

	Listener* valueChangedListener = NULL;

	uint8_t pin;
	bool isAnalog = false;

	void setup() {
		pinMode(pin, INPUT);
		if (!isAnalog) {
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

		if (valueChangedListener != NULL) {
			ValueChangedEvent event;
			event.previousValue = lastValue;
			event.currentValue = value;
			valueChangedListener->handleEvent(&event);
		}

		lastValue = value;

	}

};

#endif
