/*
 *  Author: Claudiu Matei
 */

#ifndef HttpServer_h
#define HttpServer_h

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdint.h>
#include <string.h>
#include <WString.h>

#define DEBUG_HTTP_SERVER 0

#define LINE_BUFFER_SIZE 64

class HttpServer;

class HttpCommandEvent : public Event {
public:

	const char* command;

	HttpServer* server;

	EthernetClient* client;

};

class HttpServer {
public:

	Listener* commandReceivedListener = NULL;

	void setup() {
		// set SD chip select pin to HIGH
//		pinMode(4, OUTPUT);
//		digitalWrite(4, HIGH);

		port = 80;

		static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
		static uint8_t ip[] = { 192, 168, 100, 253 };

		Ethernet.begin(mac, ip);
		this->server = new EthernetServer(this->port);
	}

	void loop() {
		// listen for incoming clients
		EthernetClient client = server->available();

		if (client) {
			char currentLine[64];
			int currentLineSize = 0;
			activeClient = &client;

			while (client.connected()) {
				if (client.available()) {
					char c = client.read();

					if (c == '\n') {
						currentLine[currentLineSize] = '\0';

						#if DEBUG_HTTP_SERVER > 0
						Serial.print(F(">> ")); Serial.println(currentLine);
						#endif

						if (currentLineSize == 0) {
							break;
						}

						if (strncmp(currentLine, "GET", 3) == 0 || strncmp(currentLine, "POST", 4 == 0)) {
							char* k = strchr(currentLine, ' ');
							k[0] = '\0'; // break string
							k+=2; // skip space and leading url slash
							char* requestUrl = k;
							k = strchr(requestUrl, ' ');
							k[0] = 0;
							dispatchEvent(currentLine, requestUrl, activeClient);
							break;
						}
					}
					else if (c != '\r' && currentLineSize < LINE_BUFFER_SIZE - 1) {
						currentLine[currentLineSize++] = c;
					}
				}
			}

			// give the web browser time to receive the data
			delay(1);
			client.stop();
		}
	}

	void httpSuccess(const char* contentType) {
		activeClient->println(F("HTTP/1.1 200 OK"));
		activeClient->print(F("Content-Type: ")); activeClient->println(contentType);
		activeClient->println(F("Access-Control-Allow-Origin: *"));
		activeClient->println(F("Connection: close"));  // the connection will be closed after completion of the response
		activeClient->println();
	}

	void dispatchEvent(const char* requestMethod, const char* requestUrl, EthernetClient* client) {

		#if DEBUG_HTTP_SERVER > 0
		Serial.print(F("HttpServer.dispatchEvent: ")); Serial.print(requestMethod); Serial.print(F(" * ")); Serial.print(requestUrl); Serial.println();
		#endif

		if (commandReceivedListener == NULL) {
			return;
		}

		HttpCommandEvent event;
		event.command = requestUrl;
		event.server = this;
		event.client = client;
		commandReceivedListener->handleEvent(&event);

	}


protected:

	int port;

	EthernetServer* server;

	EthernetClient* activeClient;

};

#endif
