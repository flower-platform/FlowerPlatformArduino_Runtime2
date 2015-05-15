/*
 *  Author: Claudiu Matei
 */

#ifndef HttpServer_h
#define HttpServer_h

#ifdef DEBUG_HttpServer
#define DB_P_HttpServer(text) Serial.print(text)
#define DB_PLN_HttpServer(text) Serial.println(text)
#else
#define DB_P_HttpServer(text)
#define DB_PLN_HttpServer(text)
#endif

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdint.h>
#include <string.h>
#include <WString.h>


#define LINE_BUFFER_SIZE 64

class HttpServer;

class HttpCommandEvent : public Event {
public:

	const char* url;

	HttpServer* server;

	EthernetClient* client;

};

class HttpServer {
public:
	static const int CONTENT_TYPE_JSON = 0;

	static const int CONTENT_TYPE_HTML = 1;

	Listener* onCommandReceived = NULL;

	int port;

	uint8_t ipAddress[4];

	uint8_t macAddress[6];

	void setup() {
		// set SD chip select pin to HIGH
//		pinMode(4, OUTPUT);
//		digitalWrite(4, HIGH);

//		static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//		static uint8_t ip[] = { 192, 168, 100, 253 };

		Ethernet.begin(macAddress, ipAddress);
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

						DB_P_HttpServer(F(">> ")); DB_PLN_HttpServer(currentLine);

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

	void httpSuccess(int contentType = CONTENT_TYPE_JSON) {
		activeClient->println(F("HTTP/1.1 200 OK"));
		activeClient->print(F("Content-Type: ")); activeClient->println(contentType == CONTENT_TYPE_HTML ? F("text/html") : F("application/json"));
		activeClient->println(F("Access-Control-Allow-Origin: *"));
		activeClient->println(F("Connection: close"));  // the connection will be closed after completion of the response
		activeClient->println();
	}

	void httpError404() {
		activeClient->println(F("HTTP/1.1 404 Not Found"));
		activeClient->println(F("Connection: close"));  // the connection will be closed after completion of the response
		activeClient->println();
	}

	void dispatchEvent(const char* requestMethod, const char* requestUrl, EthernetClient* client) {

		#if DEBUG_HTTP_SERVER > 0
		Serial.print(F("HttpServer.dispatchEvent: ")); Serial.print(requestMethod); Serial.print(F(" * ")); Serial.print(requestUrl); Serial.println();
		#endif

		if (onCommandReceived == NULL) {
			return;
		}

		HttpCommandEvent event;
		event.url = requestUrl;
		event.server = this;
		event.client = client;
		onCommandReceived->handleEvent(&event);

	}

	void getCommandFromUrl(const char* url, char* command) {
		char* p = strchr(url, '?');
		if (p == NULL) {
			strcpy(command, url);
		} else {
			uint8_t n = p - url;
			strncpy(command, url, n);
			command[n] = '\0';
		}
	}

	void getParameterValueFromUrl(const char* url, const char* param, char* value) {
		char* st = strstr_P(url, param); // look for param string start in url
		if (st != NULL) {
			st = strchr(st, '=') + 1; // look for value start
			char* en = strchr(st, '&'); // look for end of value string
			if (en == NULL) {
				strcpy(value, st);
			} else {
				uint8_t n = en - st;
				strncpy(value, st, n);
				value[n] = '\0';
			}
		} else {
			value[0]='\0';
		}
	}


protected:

	EthernetServer* server;

	EthernetClient* activeClient;

};

#endif
