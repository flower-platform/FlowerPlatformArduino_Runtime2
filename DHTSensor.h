/*
 *  Author: Claudiu Matei
 */

#ifndef DHTSensor_h
#define DHTSensor_h

#include <Arduino.h>
#include <dht.h>
#include <Print.h>
#include <stddef.h>
#include <stdint.h>
#include <WString.h>



class DHTSensor {
public:

	Listener* onTemperatureChanged = NULL;
	Listener* onHumidityChanged = NULL;

	uint8_t pin;
	unsigned int pollInterval;

	void setup() {
		lastTime = 0;
		lastHumidity = -1000;
		lastTemperature = -1000;
		pinMode(pin, INPUT);

		if (pollInterval <= 0) {
			pollInterval = 1000;
		}
	}

	void loop() {
		if ((unsigned long int)(millis() - lastTime) < pollInterval) {
			return;
		}

		lastTime = millis();

		dht dhtSensor;
		dhtSensor.read11(pin);

		if (dhtSensor.temperature != lastTemperature && onTemperatureChanged != NULL) {
			ValueChangedEvent event;
			event.currentValue = dhtSensor.temperature;
			event.previousValue = lastTemperature;
			onTemperatureChanged->handleEvent(&event);
		}
		lastTemperature = dhtSensor.temperature;

		if (dhtSensor.humidity != lastHumidity && onHumidityChanged != NULL) {
			ValueChangedEvent event;
			event.currentValue = dhtSensor.humidity;
			event.previousValue = lastHumidity;
			onHumidityChanged->handleEvent(&event);
		}
		lastHumidity = dhtSensor.humidity;
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
