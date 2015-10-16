/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NetworkAdapter_h
#define ESP8266NetworkAdapter_h

#include <avr/pgmspace.h>
#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <HttpServer.h>
#include <IPAddress.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <WString.h>

#define esp Serial1

class CharSequenceParser {
public:
	CharSequenceParser(const char* sequence) {
		this->sequence = sequence;
		this->index = 0;
		this->length = strlen(sequence);
		this->wildcardValue = '\0';
	}

	bool parseNextChar(char c) {
		if (sequence[index] != c && sequence[index] != '?' && index > 0) {
			index = 0;
		}
		if (sequence[index] == '?') {
			wildcardValue = c;
			index++;
		} else if (sequence[index] == c) {
			index++;
		}
		if (index == length) {
			index = 0;
			return true;
		}
		return false;
	}

	char wildcardValue;

protected:
	const char* sequence;
	uint8_t index;
	uint8_t length;
};

class ESP8266Client;

class ESP8266NetworkAdapter {
public:

	static const int WRITE_STATUS_READY = 0;

	static const int WRITE_STATUS_WAITING_FOR_TRANSMIT_READY = 1;

	static const int WRITE_STATUS_TRANSMIT_READY = 2;

	static const int WRITE_STATUS_WAITING_FOR_SEND_OK = 3;

	static CharSequenceParser readyParser;

	static CharSequenceParser okParser;

	static CharSequenceParser connectParser;

	static CharSequenceParser disconnectParser;

	static CharSequenceParser okToWriteParser;

	static CharSequenceParser sendOkParser;

	static CharSequenceParser ipdParser;

	const char* ssid;

	const char* password;

	HttpServer* httpServer;

	ESP8266Client* clients[4] = { };

	void setup();

	void loop();

	int readNextChar(); // reads and processes the next char from ESP8266's serial port

	int writeStatus;

	const uint8_t* writeBuffer;

	size_t writeBufferSize;


protected:

	void sendCommand(const __FlashStringHelper* cmd, const char* expectedResponse);

};

class ESP8266Client : public Client {
public:

	ESP8266Client(ESP8266NetworkAdapter* networkAdapter, uint8_t clientId) {
		this->clientId = clientId;
		this->availableBytes = 0;
		this->closed = false;
		this->networkAdapter = networkAdapter;
	}

	size_t write(uint8_t b) {
		return write(&b, 1);
	}

	size_t write(const uint8_t* buf, size_t size) {
		Serial.println("* write");
		if (closed) {
			Serial.print("* "); Serial.print(clientId); Serial.println(" Client is not connected");
			return -1;
		}

		finishActiveTransmission();

		Serial.print("* "); Serial.print(clientId); Serial.print(" Prepare to send: "); Serial.println(size);
		networkAdapter->writeBuffer = buf;
		networkAdapter->writeBufferSize = size;
		esp.print(F("AT+CIPSEND=")); esp.print(clientId); esp.print(","); esp.print(size); esp.print("\r\n");
		networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY;
		Serial.print("* "); Serial.print(clientId); Serial.println(" Waiting for transmit ready...");
		while (networkAdapter->readNextChar() == ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY);
		if (networkAdapter->writeStatus == ESP8266NetworkAdapter::WRITE_STATUS_READY) {
			return size;
		}

		esp.write(buf, size);
		Serial.print("<---- ("); Serial.print(size); Serial.println(" bytes)"); Serial.write(buf, size);
		Serial.print("* "); Serial.print(clientId); Serial.println(" Waiting for SEND OK...");
		while (networkAdapter->readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_READY);
		Serial.print("* "); Serial.print(clientId); Serial.print("<---- ("); Serial.print(size); Serial.println(" bytes sent)");
		return size;
	}

	int available() {
		return availableBytes;
	}

	int read() {
		if (availableBytes == 0) return -1;
		availableBytes--;
		while (!esp.available()); // wait for data to become available
		return esp.read();
	}

	void stop() {
		closed = true;
		Serial.print("* "); Serial.print(clientId); Serial.println(" Closing ");
		finishActiveTransmission();
		esp.print(F("AT+CIPCLOSE=")); esp.println(clientId);
	}

	uint8_t connected() {
		return !closed;
	}

	virtual ~ESP8266Client() { }

	operator bool() {
		return clientId >= 0;
	}

	// not implemented
	int connect(IPAddress ip, uint16_t port) {
		return 0;
	}
	int connect(const char *host, uint16_t port) {
		return 0;
	}
	int read(uint8_t *buf, size_t size) {
		return 0;
	}
	int peek() {
		return 0;
	}
	void flush() {	}

	int availableBytes;

	uint8_t clientId;

	bool closed;

protected:

	ESP8266NetworkAdapter* networkAdapter;

	void finishActiveTransmission() {
		if (networkAdapter->writeStatus	== ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY) {
			Serial.print("* "); Serial.print(clientId); Serial.print(" Flushing buffer... ");
			while (networkAdapter->readNextChar() == ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY);
			if (networkAdapter->writeStatus == ESP8266NetworkAdapter::WRITE_STATUS_TRANSMIT_READY) {
				esp.write(networkAdapter->writeBuffer, networkAdapter->writeBufferSize);
				Serial.print("<---- ("); Serial.print(networkAdapter->writeBufferSize); Serial.println(" bytes)");
				Serial.write(networkAdapter->writeBuffer, networkAdapter->writeBufferSize);
				while (networkAdapter->readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_READY);
				Serial.print("* "); Serial.print(clientId); Serial.print("<---- ("); Serial.print(networkAdapter->writeBufferSize); Serial.println(" bytes flushed)");
			}
		}
		if (networkAdapter->writeStatus	== ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_SEND_OK) {
			while (networkAdapter->readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_READY);
			Serial.print("* "); Serial.print(clientId); Serial.print("<---- ("); Serial.print(networkAdapter->writeBufferSize); Serial.println(" bytes flushed)");
		}
	}

};

CharSequenceParser ESP8266NetworkAdapter::readyParser("\r\nready\r\n");

CharSequenceParser ESP8266NetworkAdapter::okParser("\r\nOK\r\n");

CharSequenceParser ESP8266NetworkAdapter::connectParser("?,CONNECT");

CharSequenceParser ESP8266NetworkAdapter::disconnectParser("?,CLOSED");

CharSequenceParser ESP8266NetworkAdapter::okToWriteParser("OK\r\n> ");

CharSequenceParser ESP8266NetworkAdapter::sendOkParser("SEND OK\r\n");

CharSequenceParser ESP8266NetworkAdapter::ipdParser("+IPD,");

void ESP8266NetworkAdapter::setup() {
	esp.begin(115200);
	char c;

	Serial.println("start");
	esp.println(F("AT+RST"));
	while ((c = esp.read()) == -1 || !readyParser.parseNextChar(c));
	Serial.println("reset");

	esp.println(F("ATE0"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	esp.println(F("AT+CWMODE=1"));
	while ((c = esp.read()) != '\n');

	esp.println(F("AT+CIPMUX=1"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	esp.println(F("AT+CWDHCP=1,1"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	esp.print(F("AT+CIPSTA=\""));
	esp.print(httpServer->ipAddress[0]); esp.print(".");
	esp.print(httpServer->ipAddress[1]); esp.print(".");
	esp.print(httpServer->ipAddress[2]); esp.print(".");
	esp.print(httpServer->ipAddress[3]); esp.println("\"");
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	esp.print(F("AT+CIPSERVER=1,")); esp.print(httpServer->port); esp.println();
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	esp.print(F("AT+CWJAP=\"")); esp.print(ssid); esp.print(F("\",\"")); esp.print(password); esp.println("\""); // join AP
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

}

void ESP8266NetworkAdapter::loop() {
	while (Serial.available()) {
		esp.write(Serial.read());
	}
	while (esp.available()) {
		readNextChar();
	}
}

int ESP8266NetworkAdapter::readNextChar() {
	if (!esp.available()) {
		return writeStatus;
	}

	char c = esp.read();
	Serial.write(c);
//	Serial.print("(");
//	Serial.print((int) c);
//	Serial.print(")");
//	Serial.print("|");

	if (okToWriteParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_WAITING_FOR_SEND_OK;
	}

	if (sendOkParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_READY;
	}

	if (connectParser.parseNextChar(c)) {
		Serial.print("* Client connected: "); Serial.println(connectParser.wildcardValue - '0');
		uint8_t clientId = connectParser.wildcardValue - '0';
		ESP8266Client* client = clients[clientId];
		if (client == NULL) {
			client = new ESP8266Client(this, clientId);
			clients[clientId] = client;
		} else {
			client->closed = false;
		}
	}

	if (disconnectParser.parseNextChar(c)) {
		uint8_t clientId = disconnectParser.wildcardValue - '0';
		Serial.print("* Client disconnected: "); Serial.println(clientId);
		ESP8266Client* client = clients[clientId];
		client->closed = true;
	}

	if (ipdParser.parseNextChar(c)) {
		char params[8];
		uint8_t i = 0;
		// read +IPD params; tokenize
		while (c != ':') {
			while (!esp.available()); // wait for data to become available
			c = esp.read();
			params[i++] = (c != ',' && c != ':' ? c : '\0');
		}

		uint8_t clientId = atoi(params);
		int availableBytes = atoi(params + strlen(params) + 1);
		Serial.print("--------> "); Serial.print(clientId); Serial.print(" "); Serial.println(availableBytes);
		ESP8266Client* client = clients[clientId];
		if (client == NULL || !client->connected()) {
			while (availableBytes > 0) {
				while (!esp.available());	// wait for data
				esp.read();
				availableBytes--;
			}
			return writeStatus;
		}
		client->availableBytes = availableBytes;
		if (client->clientId < 1) {
			httpServer->processClientRequest(client, client->clientId);
			while (client->available()) { client->read(); }
		} else if (client->clientId == 1) {
			while (client->available()) { client->read(); }
			write_P(client, PSTR("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n"));
			client->stop();
		} else {
			while (client->available()) { client->read(); }
			client->stop();
		}
	}
	return writeStatus;
}

#endif
