/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

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

/*
class DataChangedEvent : public Event {
	public:
		String attribute;
		String previousValue;
		String currentValue;
};

class DataSetChangedEvent : public Event {
	public:
		std::vector<DataSetEntry> dataSet;
};

class DataSetEntry {
	public:
		DataSetEntry(String attribute, String value);
		String attribute;
		String value;
};
*/

// int EventDispatcher::nextEventType = 0;

#endif
