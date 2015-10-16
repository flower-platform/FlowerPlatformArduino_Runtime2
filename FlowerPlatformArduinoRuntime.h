/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

#include <avr/pgmspace.h>
#include <Print.h>
#include <stdint.h>

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

void write_P(Print* p, const char* s, int BUF_SIZE = 64) {
	int n = strlen_P(s);
	int k = 0;
	char buf[BUF_SIZE];
	while (k < n) {
		int l = k + BUF_SIZE <= n ? BUF_SIZE : n - k;
		memcpy_P(buf, s + k, l);
		p->write((uint8_t*) buf, l);
		k += l;
	}
}

#endif
