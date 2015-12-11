/*
 *  Author: Claudiu Matei
 */

#ifndef EthernetNetworkAdapter_h
#define EthernetNetworkAdapter_h

#include <Ethernet.h>
#include <EthernetServer.h>
#include <HttpServer.h>
#include <stdint.h>


class EthernetNetworkAdapter {
public:
	uint8_t macAddress[6];

	HttpServer* httpServer;

	void setup() {
		// Disable SPI for SD card.
		// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
		pinMode(4, OUTPUT);
		pinMode(4, HIGH);

		Ethernet.begin(macAddress, httpServer->ipAddress);
		this->server = new EthernetServer(httpServer->port);
	}

	void loop() {
		// listen for incoming clients
		EthernetClient client = server->available();
		if (client) {
			Serial.println(client);
			httpServer->processClientRequest(&client);
		}
	}

protected:

	EthernetServer* server;

};

#endif
