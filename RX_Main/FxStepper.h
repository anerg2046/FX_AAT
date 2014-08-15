#ifndef FxStepper_h
#define FxStepper_h

#include <Arduino.h>

class FxStepper {
	public:
		FxStepper(byte stepPin, byte dirPin, float speeds);
		void start();
		void rotate(long steps);
		void run();
		boolean isDone();
		long getStepsGone();
	private:
		byte stepPin;
		byte dirPin;

		float stepTime;
		float startTime;

		boolean done;
		boolean dirLast;

		int dir;

		long stepToGo;
		long stepGone;

		void step();

};
#endif