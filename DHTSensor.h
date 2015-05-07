/*
 *  Author: Claudiu Matei
 */

#ifndef DHTSensor_h
#define DHTSensor_h

#include <Arduino.h>
#include <dht.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <stddef.h>
#include <stdint.h>

class DHTSensor {
public:

	Listener* temperatureChangedListener = NULL;
	Listener* humidityChangedListener = NULL;

	uint8_t pin;
	unsigned int pollInterval;

	virtual ~DHTSensor() {
		delete dhtSensorInstance;
	}

	void setup() {
		lastTime = 0;
		previousHumidity = -1000;
		previousTemperature = -1000;
		pinMode(pin, INPUT);

		this->dhtSensorInstance = new dht();
		if (pollInterval < 0) {
			pollInterval = 0;
		}
	}

	void loop() {
		if ((unsigned long int)(millis() - lastTime) < pollInterval) {
			return;
		}

		lastTime = millis();

		check(pin);

		if (currentTemperature != previousTemperature && temperatureChangedListener != NULL) {
			ValueChangedEvent event;
			event.currentValue = currentTemperature;
			event.previousValue = previousTemperature;
			temperatureChangedListener->handleEvent(&event);
		}
		previousTemperature = currentTemperature;

		if (currentHumidity != previousHumidity && humidityChangedListener != NULL) {
			ValueChangedEvent event;
			event.currentValue = currentHumidity;
			event.previousValue = previousHumidity;
			humidityChangedListener->handleEvent(&event);
		}
		previousHumidity = currentHumidity;
	}

	void check(int pin) {
		dhtSensorInstance->read11(pin);
		currentTemperature =  dhtSensorInstance->temperature;
		currentHumidity = dhtSensorInstance->humidity;
	}

protected:
	dht* dhtSensorInstance;
	unsigned long lastTime;
	double currentTemperature;
	double currentHumidity;
	double previousTemperature;
	double previousHumidity;
};

/*
class AnalogTemperatureSensor : public Input {
	public:
        AnalogTemperatureSensor(int pin, bool isDigital, unsigned long int pollInterval, int numberOfAttributes);
		void refresh();
		double calculateAnalogTemperature(int value);
    protected:
		unsigned long int pollInterval;
		unsigned long lastTime;
		double currentTemperature;
		double previousTemperature = -1000;
		int currentValue;
};

const int EVENT_SUBTYPE_DHT_SENSOR = 2;

class DHTSensorEvent : public Event {
	public:
		Input* input;
		double digitalTemperature = 0.00;
		double digitalHumidity = 0.00;
};

const int EVENT_SUBTYPE_ANALOG_SENSOR = 3;
class AnalogTemperatureSensorEvent : public Event {
	public:
		Input* input;
		double analogTemperature;
};
 */

#endif
