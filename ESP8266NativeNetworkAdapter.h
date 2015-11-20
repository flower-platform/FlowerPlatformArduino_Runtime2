/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NativeNetworkAdapter_h
#define ESP8266NativeNetworkAdapter_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HttpServer.h>
#include <IPAddress.h>
#include <WiFiServer.h>

class ESP8266NativeNetworkAdapter {
public:
	char* ssid;

	const char* password;

	HttpServer* httpServer;

	void setup() {
		this->server = new WiFiServer(httpServer->port);
		WiFi.begin(ssid, password);
		IPAddress ip = IPAddress(httpServer->ipAddress);
		IPAddress gateway = ip;
		gateway[3] = 1;
		WiFi.config(ip, gateway, IPAddress({0xFF, 0xFF, 0xFF, 0x00 }));
		this->server->begin();
	}

	void loop() {
		// listen for incoming clients
		WiFiClient client = server->available();
		if (client) {
			int t = 0;
			while (!client.available() && t++ < 500) {
				delay(1);
			}
			httpServer->processClientRequest(&client);
		}
	}

protected:

	WiFiServer* server = NULL;

};

#endif
