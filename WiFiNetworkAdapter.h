/*
 *  Author: Claudiu Matei
 */

#ifndef WiFiNetworkAdapter_h
#define WiFiNetworkAdapter_h

#include <Arduino.h>
#include <HardwareSerial.h>
#include <HttpServer.h>
#include <stddef.h>
#include <WiFi.h>
#include <WiFiServer.h>

class WiFiNetworkAdapter {
public:
	char* ssid;

	const char* password;

	HttpServer* httpServer;

	void setup() {
		// Disable SPI for SD card.
		// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
		pinMode(4, OUTPUT);
		pinMode(4, HIGH);

		this->server = new WiFiServer(httpServer->port);
		WiFi.begin(ssid, password);
		WiFi.config(httpServer->ipAddress);
		this->server->begin();
	}

	void loop() {
		// listen for incoming clients
		WiFiClient client = server->available();
		if (client) {
			server->begin();
			httpServer->processClientRequest(&client);
		}
	}

protected:

	WiFiServer* server = NULL;

};

#endif
