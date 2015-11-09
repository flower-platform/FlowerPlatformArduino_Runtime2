/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NetworkAdapter_h
#define ESP8266NetworkAdapter_h

#include <Arduino.h>
#include <Client.h>
#include <HttpServer.h>


#define esp Serial1
#define MAX_CLIENTS 3
#define READ_BUFFER_SIZE 64
#define WRITE_TIMEOUT 3000

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

	char wildcardValue; // stores the char corresponding to the last wildcard character

protected:
	const char* sequence;
	uint8_t index;
	uint8_t length;
};

class ESP8266NetworkAdapter;

class ESP8266Client : public Client {
public:

	ESP8266NetworkAdapter* networkAdapter;

	uint8_t clientId;

	uint8_t buf[READ_BUFFER_SIZE];	// read buffer

	size_t bytesExpected;	// number of bytes promised by the ESP8266 module

	long lastRead; // timestamp of last read operation

	int bufSt, bufEn;	// start and end indexes in buffer

	bool stopped = true;	// flag set when stop() method is called; used as a mark for closing the connection

	bool closed = true;		// flag is set when a DISCONNECTED event is received (the socket is closed)

	bool accumulate(uint8_t b);	// tries to accumulate on byte in the buffer; returns whether the byte was accumulated or not (when buffer is full)

	void reset();	// resets buffer head and tail; marks the client as active (closed=false, stopped=false)

	int connect(IPAddress ip, uint16_t port);
	int connect(const char *host, uint16_t port);
	size_t write(uint8_t);
	size_t write(const uint8_t *buf, size_t size);
	int available();
	int read();
	int read(uint8_t *buf, size_t size);
	int peek();
	void flush();
	void stop();
	uint8_t connected();
	operator bool() {
		return true;
	}
	virtual ~ESP8266Client() { }
};


class ESP8266NetworkAdapter {
public:

	// write status constants
	static const int WRITE_STATUS_READY = 0;
	static const int WRITE_STATUS_WAITING_FOR_TRANSMIT_READY = 1;
	static const int WRITE_STATUS_TRANSMIT_READY = 2;
	static const int WRITE_STATUS_WAITING_FOR_SEND_OK = 3;

	// parsers for module's command responses
	static CharSequenceParser readyParser;
	static CharSequenceParser okParser;
	static CharSequenceParser okToWriteParser;
	static CharSequenceParser sendOkParser;

	// parsers for module's events
	static CharSequenceParser connectParser;
	static CharSequenceParser disconnectParser;
	static CharSequenceParser ipdParser;

	const char* ssid;

	const char* password;

	HttpServer* httpServer;

	ESP8266Client clients[MAX_CLIENTS];

	uint8_t extraClientsStack[8];	// stack holding ids of extra connections (when there are more than MAX_CLIENTS connections)

	uint8_t extraClientsSP;	// stack pointer

	ESP8266Client* recvClient;	// active receiver

	void setup();

	void loop();

	int readNextChar(); // reads a char from ESP8266 and processes it

	void closeInactiveClients();	// closes the clients marked to be closed ("stopped") and the extra connections

	int writeStatus;

};

void ESP8266Client::reset() {
	bufSt = 0;
	bufEn = 0;
	stopped = false;
	closed = false;
};

size_t ESP8266Client::write(uint8_t b) {
	return write(&b, 1);
}

size_t ESP8266Client::write(const uint8_t* buf, size_t size) {
//	Serial.println("* write");
//	if (closed) {
//		Serial.print("* "); Serial.print(clientId); Serial.println(" Client is not connected");
//		return -1;
//	}

	long t;

//	Serial.print("* "); Serial.print(clientId); Serial.print(" Prepare to send: "); Serial.println(size);
	esp.print(F("AT+CIPSEND=")); esp.print(clientId); esp.print(","); esp.print(size); esp.print("\r\n");
//	Serial.print("* "); Serial.print(clientId); Serial.println(" Waiting for transmit ready...");
	networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY;
	t = millis();
	while (networkAdapter->readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_TRANSMIT_READY && millis() < t + WRITE_TIMEOUT);


//	Serial.print("<---- ("); Serial.print(size); Serial.println(" bytes)"); Serial.write(buf, size); Serial.println();
	esp.write(buf, size);
//	Serial.print("* "); Serial.print(clientId); Serial.println(" Waiting for SEND OK...");
	t = millis();
	while (networkAdapter->readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_READY && millis() < t + WRITE_TIMEOUT);
	networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_READY;
//	Serial.print("* "); Serial.print(clientId); Serial.print("<---- ("); Serial.print(size); Serial.println(" bytes sent)");
	return size;
}

int ESP8266Client::available() {
	return bufEn - bufSt;
}

int ESP8266Client::read() {
	if (bufSt == bufEn) {
		return -1;
	}
	return buf[bufSt++];
}

bool ESP8266Client::accumulate(uint8_t b) {
	bytesExpected--;
	if (bufSt == bufEn) {
		// reset buffer indexes
		bufSt = 0;
		bufEn = 0;
	}
	if (bufEn < READ_BUFFER_SIZE) {
		buf[bufEn++] = b;
		return true;
	}
	return false;
}

void ESP8266Client::stop() {
	stopped = true;
	Serial.print("* "); Serial.print(clientId); Serial.println(" stopped");
}

uint8_t ESP8266Client::connected() {
	return !closed;
}

// not implemented
int ESP8266Client::connect(IPAddress ip, uint16_t port) {
	return 0;
}
int ESP8266Client::connect(const char *host, uint16_t port) {
	return 0;
}
int ESP8266Client::read(uint8_t *buf, size_t size) {
	return 0;
}
int ESP8266Client::peek() {
	return 0;
}
void ESP8266Client::flush() {	}


CharSequenceParser ESP8266NetworkAdapter::readyParser("\r\nready\r\n");

CharSequenceParser ESP8266NetworkAdapter::okParser("OK\r\n");

CharSequenceParser ESP8266NetworkAdapter::connectParser("?,CONNECT");

CharSequenceParser ESP8266NetworkAdapter::disconnectParser("?,CLOSED");

CharSequenceParser ESP8266NetworkAdapter::okToWriteParser("OK\r\n> ");

CharSequenceParser ESP8266NetworkAdapter::sendOkParser("SEND OK\r\n");

CharSequenceParser ESP8266NetworkAdapter::ipdParser("+IPD,");

void ESP8266NetworkAdapter::setup() {
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		clients[i].clientId = i;
		clients[i].networkAdapter = this;
	}
	extraClientsSP = 0;

	esp.begin(115200);
	char c;

	Serial.println("start");

	// reset ESP8266
	esp.println(F("AT+RST"));
	while ((c = esp.read()) == -1 || !readyParser.parseNextChar(c));
	Serial.println("reset");

	// echo off
	esp.println(F("ATE0"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// set WiFi mode to client only
	esp.println(F("AT+CWMODE=1"));
	while ((c = esp.read()) != '\n');

	// set mux mode to multi
	esp.println(F("AT+CIPMUX=1"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// set server timeout (3s)
	esp.println(F("AT+CIPSTO=3"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// disable DHCP
	esp.println(F("AT+CWDHCP=1,1"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// set static IP address
	esp.print(F("AT+CIPSTA=\""));
	esp.print(httpServer->ipAddress[0]); esp.print(".");
	esp.print(httpServer->ipAddress[1]); esp.print(".");
	esp.print(httpServer->ipAddress[2]); esp.print(".");
	esp.print(httpServer->ipAddress[3]); esp.println("\"");
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// enable "server" mode
	esp.print(F("AT+CIPSERVER=1,")); esp.print(httpServer->port); esp.println();
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// join AP
	esp.print(F("AT+CWJAP=\"")); esp.print(ssid); esp.print(F("\",\"")); esp.print(password); esp.println("\"");
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

}

void ESP8266NetworkAdapter::loop() {
	while (Serial.available()) {
		esp.write(Serial.read());
	}
	while (esp.available()) {
		readNextChar();
		if (!esp.available()) {
			// give some time to get over possible congestion
			delay(10);
		}
	}

	closeInactiveClients();

	// dispatch events
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		ESP8266Client* client = &clients[i];
		if (client->available()) {
			if (client->connected()) {
				httpServer->processClientRequest(client);
			} else if (!client->stopped) {
				client->stop();
			}
		}
	}
}

int ESP8266NetworkAdapter::readNextChar() {
	if (!esp.available()) {
		return writeStatus;
	}

	char c = esp.read();
	Serial.write(c);

	if (okToWriteParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_TRANSMIT_READY;
	}

	if (sendOkParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_READY;
		closeInactiveClients();
	}

	if (connectParser.parseNextChar(c)) {
		uint8_t clientId = connectParser.wildcardValue - '0';
		Serial.print("* Client connected: "); Serial.println(clientId);
		if (clientId < MAX_CLIENTS) {
			ESP8266Client* client = &clients[clientId];
			client->reset();
		} else {
			extraClientsStack[extraClientsSP++] = clientId;
		}
	}

	if (disconnectParser.parseNextChar(c)) {
		uint8_t clientId = disconnectParser.wildcardValue - '0';
		Serial.print("* Client disconnected: "); Serial.println(clientId);
		if (clientId < MAX_CLIENTS) {
			ESP8266Client* client = &clients[clientId];
			client->closed = true;
		}
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
		Serial.print("--------> "); Serial.print(clientId); Serial.print(" "); Serial.print(availableBytes); Serial.print(" "); Serial.println(writeStatus);
		ESP8266Client* client = clientId < MAX_CLIENTS ? &clients[clientId] : NULL;
		if (client) {
			recvClient = client;
			recvClient->bytesExpected = availableBytes;
			while (!esp.available());	// wait for next char
			c = esp.read();
		}
	}

	if (recvClient != NULL) {
		if (!recvClient->accumulate(c) || recvClient->bytesExpected <= 0) {
			recvClient = NULL;
		}
	}

	return writeStatus;
}

void ESP8266NetworkAdapter::closeInactiveClients() {
	// Serial.println("closeClients()");

	// Close stopped clients
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].stopped && !clients[i].closed) {
			Serial.print("Closing client "); Serial.println(i);
			delay(20);
			write_P(&esp, PSTR("AT+CIPCLOSE=")); esp.println(i);
			delay(20);
			clients[i].closed = true;
			Serial.print("Closed client "); Serial.println(i);
		}
	}

	// Close extra clients
	while (extraClientsSP > 0) {
		uint8_t clientId = extraClientsStack[--extraClientsSP];
		Serial.print("Closing client "); Serial.println(clientId);
		delay(20);
		write_P(&esp, PSTR("AT+CIPCLOSE=")); esp.println(clientId);
		delay(20);
		Serial.print("Closed client "); Serial.println(clientId);
	}
}

#endif
