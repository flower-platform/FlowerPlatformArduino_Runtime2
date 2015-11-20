#ifndef Input_h
#define Input_h

#include <Arduino.h>
#include <stdbool.h>



class Input {
protected:
	int lastValue;
	unsigned long lastTime;

public:

	Listener* onValueChanged = NULL;

	uint8_t pin;
	int pollInterval = 200;
	bool internalPullUp = false;
	bool isAnalog = false;

	void setup() {
		pinMode(pin, INPUT);
		if (internalPullUp) {
			digitalWrite(pin, HIGH);
			lastValue = HIGH;
		} else {
			lastValue = LOW;
		}
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
		if (!isAnalog && (unsigned long int)(millis() - lastTime) < pollInterval) {
			return;
		}

		if (onValueChanged != NULL) {
			ValueChangedEvent event;
			event.previousValue = lastValue;
			event.currentValue = value;
			onValueChanged->handleEvent(&event);
		}

		lastValue = value;

		if (!isAnalog) {
			lastTime = millis();
		}

	}

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
		print->print(F("\""));
		print->print(instanceName);
		print->print(F("\":"));
		print->print(lastValue);
	}

};

#endif
