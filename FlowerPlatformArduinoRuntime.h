/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

#include <Print.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

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
#ifdef ESP8266
	return system_get_free_heap_size();
#else
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#endif
}

size_t write_P(Print* p, const char* s) {
	size_t n = strlen_P(s);
	char buf[n];
	memcpy_P(buf, s, n);
	p->write((uint8_t*) buf, n);
	return n;
}

#define BUFFERED_PRINT_BUFFER_SIZE 256
class BufferedPrint : public Print {
public:

	BufferedPrint(Print* print) {
		this->out = print;
		this->bufIndex = 0;
	}

	size_t write(uint8_t b) {
		return write(&b, 1, false);
	}

	size_t write(const uint8_t* buffer, size_t size) {
		return write(buffer, size, false);
	}

	size_t write_P(const char* s) {
		return write((uint8_t*) s, strlen_P(s), true);
    }

	size_t write_P(const uint8_t* buffer, size_t size) {
		return write(buffer, size, true);
    }

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem) {
    	size_t n = size;
    	size_t writableBytes;
    	do {
    		writableBytes = bufIndex + n > BUFFERED_PRINT_BUFFER_SIZE ? BUFFERED_PRINT_BUFFER_SIZE - bufIndex : n;
    		if (isProgMem) {
        		memcpy_P(buf + bufIndex, buffer, writableBytes);
    		} else {
        		memcpy(buf + bufIndex, buffer, writableBytes);
    		}
    		bufIndex += writableBytes;
    		buffer += writableBytes;
    		n -= writableBytes;
    		if (bufIndex == BUFFERED_PRINT_BUFFER_SIZE) {
    			flush();
    		}
    	} while (n > 0);
    	return size;
    }

//    size_t write(const uint8_t *buffer, size_t size) {
//    	if (bufIndex + size > BUFFERED_PRINT_BUFFER_SIZE) {
//    		flush();
//    	}
//    	if (size > BUFFERED_PRINT_BUFFER_SIZE) {
//    		out->write(buffer, size);
//    	} else {
//    		memcpy(buf + bufIndex, buffer, size);
//    		bufIndex += size;
//    	}
//    	return size;
//    }

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
