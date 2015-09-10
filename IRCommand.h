/*
 *  Author: Claudiu Matei
 */

#ifndef IRCommand_h
#define IRCommand_h

#include <Arduino.h>
#include <IRremote.h>
#include <pins_arduino.h>
#include <stdlib.h>
#include <SD.h>

/**
 * Uses pin 2 for PWM
 */
class IRCommand {
public:
	const char* name;

	void setup() {
//		pinMode(SS, OUTPUT);
//		digitalWrite(SS, HIGH);
		SD.begin(4);
	}

	void send() {
//		Serial.println(name);
		File f = SD.open(name);
		char buf[6];
		unsigned int* command = NULL;
		int n = 0, i = 0;
		while (f.available()) {
			char c = f.read();
			if (c=='\r') {
				continue;
			} else  if (c == '\n') {
				buf[i] = '\0';
				unsigned int k = atoi(buf);
				i = 0;
				if (command == NULL) { // first line; length of sequence
					command = new unsigned int[k];
				} else {
					command[n++] = k;
				}

			}
			buf[i++] = c;
		}
		f.close();
//		Serial.println(n);
		IRsend irsend;
		const int khz = 38;
		irsend.sendRaw(command, n, khz);
		delete command;
	}

};

#endif
