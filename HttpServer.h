/*
 *  Author: Claudiu Matei
 */

#ifndef HttpServer_h
#define HttpServer_h

#include <avr/pgmspace.h>
#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <stdint.h>
#include <string.h>
#include <WString.h>

#define DEBUG_HttpServer
#ifdef DEBUG_HttpServer
#define DB_P_HttpServer(text) Serial.print(text)
#define DB_PLN_HttpServer(text) Serial.println(text)
#else
#define DB_P_HttpServer(text)
#define DB_PLN_HttpServer(text)
#endif

#define LINE_BUFFER_SIZE 64

class HttpServer;

class HttpCommandEvent : public Event {
public:

	const char* url;

	HttpServer* server;

	Client* client;

};

class HttpServer {
public:
	static const int CONTENT_TYPE_JSON = 0;

	static const int CONTENT_TYPE_HTML = 1;

	Listener* onCommandReceived = NULL;

	uint8_t ipAddress[4];

	int port;

	void setup() {

	}

	void processClientRequest(Client* client, uint8_t clientId) {
		activeClient = client;

		DB_PLN_HttpServer("data received");

		if (lineBuff[clientId] == NULL) {
			lineBuff[clientId] = new char[LINE_BUFFER_SIZE];
			lineBuffSize[clientId] = 0;
			DB_PLN_HttpServer("lineBuff allocated");
		}
		char* currentLine = lineBuff[clientId];

		while (client->connected() && client->available()) {
			char c = client->read();
			Serial.print('*'); Serial.print(c);
			if (c == '\n') {
				currentLine[lineBuffSize[clientId]] = '\0';

				DB_P_HttpServer(F(">> ")); DB_PLN_HttpServer(currentLine);

				lineBuffSize[clientId] = 0;

				if (strncmp(currentLine, "GET", 3) == 0 || strncmp(currentLine, "POST", 4 == 0)) {
					// empty input buffer
					while (client->available()) {
						client->read();
					}
					char* k = strchr(currentLine, ' ');
					k[0] = '\0'; // break string
					k+=2; // skip space and leading url slash
					char* requestUrl = k;
					k = strchr(requestUrl, ' ');
					k[0] = 0;
					DB_PLN_HttpServer(F("httpserver dispatch... "));
					dispatchEvent(currentLine, requestUrl, activeClient);
					delete lineBuff[clientId];
					lineBuff[clientId] = NULL;
					break;
				}
			}
			else if (c != '\r' && lineBuffSize[clientId] < LINE_BUFFER_SIZE - 1) {
				currentLine[lineBuffSize[clientId]++] = c;
			}
		}


		if (lineBuff[clientId] == NULL) {
			DB_PLN_HttpServer(F("closing... "));
			// empty input buffer
			while (client->available()) {
				client->read();
			}
			client->stop();
			DB_P_HttpServer(F("closed: ")); DB_PLN_HttpServer(client->connected());
		}
	}

	void httpSuccess(int contentType = CONTENT_TYPE_JSON) {
		write_P(activeClient, PSTR("HTTP/1.1 200 OK\r\nContent-Type: ")); write_P(activeClient, contentType == CONTENT_TYPE_HTML ? PSTR("text/html") : PSTR("application/json"));
		write_P(activeClient, PSTR("\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n"));  // the connection will be closed after completion of the response
	}

	void httpError404() {
		write_P(activeClient, PSTR("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n"));  // the connection will be closed after completion of the response
	}

	void dispatchEvent(const char* requestMethod, const char* requestUrl, Client* client) {

		DB_P_HttpServer(F("HttpServer.dispatchEvent: ")); DB_P_HttpServer(requestMethod); DB_P_HttpServer(F(" * ")); DB_P_HttpServer(requestUrl); DB_PLN_HttpServer();

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
		char* st = strstr(url, param); // look for param string start in url
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

	Client* activeClient;

	char* lineBuff[8];

	int lineBuffSize[8];

};

#endif
