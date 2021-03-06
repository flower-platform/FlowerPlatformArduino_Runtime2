/*
 *  Author: Claudiu Matei
 */

#ifndef IRCommand_h
#define IRCommand_h

#include <IRremote.h>
#include <SdFat.h>

class IRCommand {
public:
	const char* name;

	void setup() {
		digitalWrite(4, HIGH);
	}

	void send() {
		SdFat SD;
		SD.begin(4);
		SD.chdir("ir-commands", true);
		File f = SD.open(name);
//		Serial.println(name);
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

		// send through IR led
		IRsend irsend;
		const int khz = 38;
		irsend.sendRaw(command, n, khz);
		delete command;
	}

};

#endif
