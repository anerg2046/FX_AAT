#include "FxStepper.h"

FxStepper::FxStepper(byte stepPin, byte dirPin, float speeds)
{
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->stepTime = 1000.0 * 1000.0 / speeds;
}

void FxStepper::start()
{
    pinMode(this->stepPin, OUTPUT);
    pinMode(this->dirPin, OUTPUT);
    this->done = true;
    this->dir = 1;
    this->dirLast = 1;
    this->stepToGo = 0;
    this->stepGone = 0;
}

void FxStepper::rotate(long steps)
{
    if (steps != 0)
    {

        if (this->done != true)
        {
            this->stepToGo = (abs(this->stepToGo) - this->stepGone) * this->dir + steps;
        }
        else
        {
            this->stepToGo = steps;
            this->done = false;
        }

        if (this->stepToGo > 0)
        {
            digitalWrite(this->dirPin, HIGH);
            this->dir = 1;
        }
        else
        {
            digitalWrite(this->dirPin, LOW);
            this->dir = -1;
        }
        this->stepGone = 0;
        this->step();
    }

}

void FxStepper::run()
{

    if (this->done == false)
    {
        this->step();
    }
}

void FxStepper::step()
{
    if (abs(this->stepToGo) <= this->stepGone)
    {
        this->done = true;
    }
    else
    {
        this->startTime = micros();
        if (this->dirLast != this->dir)
        {
            while (micros() - this->startTime < this->stepTime);
            this->dirLast = this->dir;
            this->startTime = micros();
        }
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(3);
        digitalWrite(stepPin, LOW);
        while (micros() - this->startTime < this->stepTime);
        this->stepGone++;
    }
}

long FxStepper::getStepsGone()
{
    return this->stepGone * this->dir;
}

boolean FxStepper::isDone()
{
    return this->done;
}