/*
 *  Author: Claudiu Matei
 */

#ifndef HttpServer_h
#define HttpServer_h

#include <avr/pgmspace.h>
#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdint.h>
#include <string.h>

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

	void processClientRequest(Client* client) {
		char currentLine[64];
		int currentLineSize = 0;
		activeClient = client;

		DB_P_HttpServer("client connected");

		while (client->connected()) {
			if (client->available()) {
				char c = client->read();

				if (c == '\n') {
					currentLine[currentLineSize] = '\0';

					DB_P_HttpServer(F(">> ")); DB_PLN_HttpServer(currentLine);

					if (currentLineSize == 0) {
						break;
					}
					currentLineSize = 0;

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


		DB_PLN_HttpServer(F("closing... "));
		// empty input buffer
		while (client->available()) {
			client->read();
		}
		client->stop();
		DB_P_HttpServer(F("closed: ")); DB_PLN_HttpServer(client->connected());
	}

	void httpSuccess(int contentType = CONTENT_TYPE_JSON) {
		write(PSTR("HTTP/1.1 200 OK\r\n"));
		write(PSTR("Content-Type: ")); write(contentType == CONTENT_TYPE_HTML ? PSTR("text/html") : PSTR("application/json"));
		write(PSTR("\r\nAccess-Control-Allow-Origin: *\r\n"));
		write(PSTR("Connection: close\r\n\r\n"));  // the connection will be closed after completion of the response
	}

	void httpError404() {
		write(PSTR("HTTP/1.1 404 Not Found\r\n"));
		write(PSTR("Connection: close\r\n\r\n"));  // the connection will be closed after completion of the response
	}

	void dispatchEvent(const char* requestMethod, const char* requestUrl, Client* client) {

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

	void write(const char* s) {
		int n = strlen_P(s);
		int k = 0;
		char buf[64];
		while (k < n) {
			int l = k + 64 <= n ? 64 : n - k;
			memcpy_P(buf, s + k, l);
			activeClient->write((uint8_t*) buf, l);
			k += l;
		}
	}


protected:

	Client* activeClient;

};

#endif
