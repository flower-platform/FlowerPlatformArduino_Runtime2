#ifndef LoggerSD_h
#define LoggerSD_h

#include <Arduino.h>
#include <pins_arduino.h>
#include <stdbool.h>
#include <stdint.h>
#include <SD.h>
#include <WString.h>


template <class T> class LoggerSD {
protected:
	unsigned long lastTimestamp;

public:

	uint8_t slaveSelectPin;

	const char* fileName;

	unsigned long timeInterval;

	bool valueReady;

	T lastValue;

	void setup() {
		lastTimestamp = 0;
		if (timeInterval == 0) {
			timeInterval = 60 * 1000L;
		}
		valueReady = false;
		pinMode(SS, OUTPUT);
		pinMode(SS, HIGH);
		pinMode(slaveSelectPin, OUTPUT);
		pinMode(slaveSelectPin, HIGH);
		SD.begin(slaveSelectPin);
	}

	void loop() {
		unsigned long t = millis();
		if (!valueReady || t < lastTimestamp + timeInterval) {
			return;
		}

		lastTimestamp = t;

		File file = SD.open(fileName, FILE_WRITE);
//		Serial.print(F("Writting... ")); Serial.print(lastValue); Serial.print(" "); Serial.println(fileName);
		if (file) {
			file.print(t);
			file.print(F(","));
			file.println(lastValue);
			file.close();
//			Serial.println(F("Written"));
		}
		valueReady = false;

	}

	void log(T value) {
//		Serial.print(F("Logging... ")); Serial.print(value); Serial.print(" "); Serial.println(fileName);
		lastValue = value;
		valueReady = true;
	}

/*
	void readLog() {
		if (!fileName) {
			fileName = new char[9];
			int cnLength = channelName.length();
			String fn;
			if (channelName.length() <= 8) {
				fn = fileName;
				channelName.toCharArray(fileName, cnLength, 0);
			} else {
				String fn = channelName.substring(0, 4) + "~";
				fn += channelName.substring(cnLength - 3);
				fn.toCharArray(fileName, cnLength, 0);
			}
			Serial.println(fileName);
		}

		ArduinoOutStream cout(Serial);

		if (file.open(fileName, O_READ)) {
			while (file.dirSize())
			file.read()
			file.close();
		}
	}
*/

};

#endif
