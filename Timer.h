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
	Listener* timerListener = NULL;
	Listener* timerCompleteListener = NULL;

	unsigned int currentCount;

	unsigned long delay;

	unsigned int repeatCount;

	bool running;

	Timer(bool autostart) {
		repeatCount = 0;
		running = autostart;
		lastTimestamp = 0;
		currentCount = 0;
		delay = 1000;
	}

	void loop() {
		if (!running) {
			return;
		}
		if (repeatCount > 0 && currentCount > repeatCount) {
			running = false;
			return;
		}
		if (millis() > lastTimestamp + delay) {
			currentCount++;

			TimerEvent event;
			if (timerListener != NULL) {
				timerListener->handleEvent(&event);
			}

			if (repeatCount > 0 && currentCount == repeatCount) {
				if (timerCompleteListener != NULL) {
					timerCompleteListener->handleEvent(&event);
				}
				running = false;
			}
			lastTimestamp = millis();
		}

	}

	void reset() {
		running = false;
		currentCount = 0;
	}

	void start() {
		lastTimestamp = millis();
		running = true;
	}

	void stop() {
		running = false;
	}

};

#endif
