#ifndef Logger_h
#define Logger_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>
#include <stdint.h>
#include <SD.h>


class Logger {
protected:
	unsigned long lastTimestamp;

public:

//	String channelName;

	char* fileName;

	unsigned long timeInterval;

	Logger(uint8_t chipSelectPin) {
		lastTimestamp = 0;
		fileName = "logger.txt";
		timeInterval = 60 * 1000;
		pinMode(SS, OUTPUT);
		digitalWrite(SS, HIGH);
		pinMode(chipSelectPin, OUTPUT);
		pinMode(chipSelectPin, HIGH);
		SD.begin(chipSelectPin);
	}

	void log(int value) {
		unsigned long t = millis();
		if (t < lastTimestamp + timeInterval) {
			return;
		}
		File file = SD.open(fileName, FILE_WRITE);
		if (file) {
			file.print(t);
			file.print(",");
			file.println(value);
			file.close();
		}
		Serial.print("LOG ");
		Serial.print(t);
		Serial.print(",");
		Serial.println(value);
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
