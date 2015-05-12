#ifndef Timer_h
#define Timer_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdbool.h>
#include <stddef.h>

class Timer;

class TimerEvent : public Event {
public:
	Timer* timer;

};

class Timer {
protected:
	unsigned long lastTimestamp;

public:
	Listener* onTimer = NULL;
	Listener* onTimerComplete = NULL;

	unsigned int currentCount;

	unsigned long delay;

	unsigned int repeatCount;

	bool autostart;

	void setup() {
		repeatCount = 0;
		lastTimestamp = 0;
		currentCount = 0;
		delay = 1000;
	}

	void loop() {
		if (!autostart) {
			return;
		}
		if (repeatCount > 0 && currentCount > repeatCount) {
			autostart = false;
			return;
		}
		if (millis() > lastTimestamp + delay) {
			currentCount++;

			TimerEvent event;
			if (onTimer != NULL) {
				onTimer->handleEvent(&event);
			}

			if (repeatCount > 0 && currentCount == repeatCount) {
				if (onTimerComplete != NULL) {
					onTimerComplete->handleEvent(&event);
				}
				autostart = false;
			}
			lastTimestamp = millis();
		}

	}

	void reset() {
		autostart = false;
		currentCount = 0;
	}

	void start() {
		lastTimestamp = millis();
		autostart = true;
	}

	void stop() {
		autostart = false;
	}

};

#endif
