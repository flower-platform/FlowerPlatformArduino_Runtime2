/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

#include <avr/pgmspace.h>
#include <Print.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define DEBUG_FP 0

class Event { };

class Listener {
public:

	virtual ~Listener() { }

	virtual void handleEvent(Event* event) { }

};

template <class T> class DelegatingListener : public Listener {
protected:
	T* instance;
	void (T::*functionPointer)(Event* event);
public:
	DelegatingListener(T* _instance, void (T::*_functionPointer)(Event* event)) {
		instance = _instance;
		functionPointer = _functionPointer;
	}
	virtual void handleEvent(Event* event) {
		(*instance.*functionPointer)(event);
	}
};

class ValueChangedEvent : public Event {
public:

	int previousValue;

	int currentValue;

};

int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

size_t write_P(Print* p, const char* s, int BUF_SIZE = 64) {
	size_t n = strlen_P(s);
	size_t k = 0;
	char buf[BUF_SIZE];
	while (k < n) {
		size_t l = k + BUF_SIZE <= n ? BUF_SIZE : n - k;
		memcpy_P(buf, s + k, l);
		p->write((uint8_t*) buf, l);
		k += l;
	}
	return n;
}

size_t write(Print* p, uint8_t* s, size_t size, int BUF_SIZE = 64) {
	size_t k = 0;
	while (k < size) {
		size_t l = k + BUF_SIZE <= size ? BUF_SIZE : size - k;
		p->write(s + k, l);
		k += l;
	}
	return size;
}


#define BUFFERED_PRINT_BUFFER_SIZE 128
class BufferedPrint : public Print {
public:

	BufferedPrint(Print* print) {
		this->out = print;
		this->bufIndex = 0;
	}

	size_t write(uint8_t b) {
		return write(&b, 1);
	}

    size_t write(const uint8_t *buffer, size_t size) {
//    	Serial.print("BP Write "); Serial.println(size);
    	if (bufIndex + size > BUFFERED_PRINT_BUFFER_SIZE) {
    		flush();
    	}
    	if (size > BUFFERED_PRINT_BUFFER_SIZE) {
    		out->write(buffer, size);
    	} else {
    		memcpy(buf + bufIndex, buffer, size);
    		bufIndex += size;
    	}
    	return size;
    }

    void flush() {
    	if (bufIndex == 0) {
    		return;
    	}
    	out->write(buf, bufIndex);
    	bufIndex = 0;
    }

    virtual ~BufferedPrint() { }

private:
    Print* out;
    uint8_t buf[BUFFERED_PRINT_BUFFER_SIZE];
    size_t bufIndex;
};

#endif
