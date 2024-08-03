#include "ChetchServo.h"

#if defined(ARDUINO_AVR_MEGA2560)
    #define TIMER_NUMBER 3
    #define TIMER_PRESCALER 8
#else
    #define TIMER_NUMBER 0
    #define TIMER_PRESCALER 0
#endif

namespace Chetch{
#if TIMER_NUMBER == 3
    ISR(TIMER3_COMPA_vect){
        Servo::handleTimerInterrupt();
    }
#endif

    ISRTimer* Servo::timer = NULL;
    byte Servo::instanceIndex = 0;
    byte Servo::currentInstance = 0; //current instance for reading ISR
    Servo* Servo::instances[Servo::MAX_INSTANCES];

    Servo* Servo::create(ServoModel servoModel){
        if(instanceIndex >= MAX_INSTANCES || TIMER_NUMBER <= 0){
            return NULL;
        } else {
            if(instanceIndex == 0){
                cli();
                timer = ISRTimer::create(TIMER_NUMBER, TIMER_PRESCALER, ISRTimer::TimerMode::COMPARE);
                timer->setCompareA(0, 0);
                sei();
            }

            Servo* instance = new Servo(servoModel);
            instances[instanceIndex++] = instance;
            return instance;
        }
    }

    void Servo::destroy(Servo* servo) {
        if (servo->attached()) {
            servo->detach();
        }
        int idx2destroy = -1;
        for (int i = 0; i < instanceIndex; i++) {
            if (instances[i] == servo) {
                idx2destroy = i;
                break;
            }
        }

        if (idx2destroy >= 0) {
            delete instances[idx2destroy];
            for (int i = idx2destroy; i < instanceIndex - 1; i++) {
                instances[i] = instances[i + 1];
            }
            instanceIndex--;
            if (currentInstance > idx2destroy) {
                currentInstance--;
            }
        }
    }

    void Servo::handleTimerInterrupt(){
        static Servo* servo = instances[currentInstance];


        if(servo->servoPulse == LOW){
            servo->servoPulse = HIGH;
            digitalWrite(servo->servoPin, servo->servoPulse);
    
            //set next ISR
            Servo::timer->setCompareA(0, servo->pulseEndInTicks);
        } else {
            servo->servoPulse = LOW;
            digitalWrite(servo->servoPin, servo->servoPulse);
    
            //set next ISR
            Servo::timer->setCompareA(0, servo->pulseStartInTicks);
        }
    }

    Servo::Servo(ServoModel model){
        this->model = model;
        switch(this->model){
            case ServoModel::MG996:
                min = 700;
                max = 2300;
                rotationSpeed = 3000; //micros to move one degree
                break;
        }
    }
  
    void Servo::attach(byte pinNumber){
        servoPin = pinNumber;
        attachedToPin = true;
    }

    void Servo::detach(){
        attachedToPin = false;
        timer->disable();
    }

    bool Servo::attached(){
        return attachedToPin;
    }

    void Servo::setTrim(int trimFactor, bool usingDegrees){
        if(usingDegrees){ //convert
           trimFactor = (int)((double)(trimFactor*(max - min)) / 180.0);
        }
        this->trimFactor = trimFactor;
    }

    void Servo::writeMicroseconds(unsigned long microseconds){
        if(!attached())return;

        microseconds += trimFactor;

        if(microseconds < min)microseconds = min;
        if(microseconds > max)microseconds = max;

        servoPulse = LOW;
        timer->disable();
  
        pulseEndInTicks = timer->microsToTicks(microseconds);
        pulseStartInTicks = timer->microsToTicks(20000 - microseconds);

        //Serial.print("Setting pulse end in ticks to: "); Serial.println(pulseEndInTicks);
        //Serial.print("Setting pulse start in ticks to: "); Serial.println(pulseStartInTicks);

        //reset counter and compare, then enable
        timer->setCompareA(0, 0);
        timer->enable();
    }


    int Servo::write(int pos){
        if(pos < 0)pos = 0;
        if(pos > 180)pos = 180;
        
        if(position != -1){
            if(isMoving()){
                position = read();
            }
            direction = pos > position ? 1 : -1;
            startedMoving = micros();
            moveDuration = rotationSpeed * (unsigned long)abs(position - pos);
        }
        unsigned long microseconds = map(pos, 0, 180, min, max);
        writeMicroseconds(microseconds);
        position = pos;
        return position;
    }

    int Servo::read(){
        if(isMoving()){
            return position - direction*((moveDuration - (micros() - startedMoving)) / rotationSpeed);
        } else {
            return position;
        }
    }

    bool Servo::isMoving(){
        if(micros() - startedMoving >= moveDuration){
            moveDuration = 0;
            return false;
        } else {
            return true;
        }
    }
} //end namespace
