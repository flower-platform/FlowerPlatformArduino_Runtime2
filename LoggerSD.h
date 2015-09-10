#ifndef LoggerSD_h
#define LoggerSD_h

#include <Arduino.h>
#include <pins_arduino.h>
#include <stdbool.h>
#include <stdint.h>
#include <SD.h>
#include <WString.h>

#ifdef DEBUG_LoggerSD
#define DB_P_LoggerSD(text) Serial.print(text)
#define DB_PLN_LoggerSD(text) Serial.println(text)
#else
#define DB_P_LoggerSD(text)
#define DB_PLN_LoggerSD(text)
#endif

template <class T> class LoggerSD {
protected:
	unsigned long lastTimestamp;
	bool valueReady;
	T lastValue;

public:

	uint8_t slaveSelectPin;

	const char* fileName;

	unsigned long timeInterval;

	void setup() {
		lastTimestamp = 0;
		if (timeInterval == 0) {
			timeInterval = 60 * 1000L;
		}
		valueReady = false;
//		pinMode(SS, OUTPUT);
//		digitalWrite(SS, HIGH);
//		pinMode(slaveSelectPin, OUTPUT);
//		digitalWrite(slaveSelectPin, HIGH);
		SD.begin(slaveSelectPin);
	}

	void loop() {
		unsigned long t = millis();
		if (!valueReady || t < lastTimestamp + timeInterval) {
			return;
		}

		lastTimestamp = t;

		DB_P_LoggerSD(F("Writting... ")); DB_P_LoggerSD(lastValue); DB_P_LoggerSD(F(" to ")); DB_PLN_LoggerSD(fileName);
		File file = SD.open(fileName, FILE_WRITE);
		if (file) {
			file.print(t);
			file.print(F(","));
			file.println(lastValue);
			file.close();
			DB_PLN_LoggerSD(F("Written"));
		}
		valueReady = false;

	}

	void log(T value) {
		DB_P_LoggerSD(F("Logging... ")); DB_P_LoggerSD(value); DB_P_LoggerSD(F(" ")); DB_PLN_LoggerSD(fileName);
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
