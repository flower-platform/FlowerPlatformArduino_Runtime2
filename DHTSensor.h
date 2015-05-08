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
		delete dhtSensor;
	}

	void setup() {
		lastTime = 0;
		lastHumidity = -1000;
		lastTemperature = -1000;
		pinMode(pin, INPUT);

		this->dhtSensor = new dht();
		if (pollInterval < 0) {
			pollInterval = 0;
		}
	}

	void loop() {
		if ((unsigned long int)(millis() - lastTime) < pollInterval) {
			return;
		}

		lastTime = millis();

		dhtSensor->read11(pin);

		if (dhtSensor->temperature != lastTemperature && temperatureChangedListener != NULL) {
			ValueChangedEvent event;
			event.currentValue = dhtSensor->temperature;
			event.previousValue = lastTemperature;
			temperatureChangedListener->handleEvent(&event);
		}
		lastTemperature = dhtSensor->temperature;

		if (dhtSensor->humidity != lastHumidity && humidityChangedListener != NULL) {
			ValueChangedEvent event;
			event.currentValue = dhtSensor->humidity;
			event.previousValue = lastHumidity;
			humidityChangedListener->handleEvent(&event);
		}
		lastHumidity = dhtSensor->humidity;
	}

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
		print->print(F("\""));
		print->print(instanceName);
		print->print(F("_temperature"));
		print->print(F("\":"));
		print->print(lastTemperature);

		print->print(F(","));

		print->print(F("\""));
		print->print(instanceName);
		print->print(F("_humidity"));
		print->print(F("\":"));
		print->print(lastHumidity);
	}


protected:
	dht* dhtSensor;
	unsigned long lastTime;
	double lastTemperature;
	double lastHumidity;
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
