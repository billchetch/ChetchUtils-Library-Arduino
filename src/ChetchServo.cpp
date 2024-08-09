#include "ChetchServo.h"

#if defined(ARDUINO_AVR_MEGA2560)
    #define TIMER_NUMBER 3
    #define TIMER_PRESCALER 8 //'ticks'every 0.5 microseconds
#else
    #define TIMER_NUMBER 0
    #define TIMER_PRESCALER 0
#endif

namespace Chetch{

    ISRTimer* Servo::timer = NULL;
    byte Servo::instanceCount = 0;
    byte Servo::currentInstance = 0; //current instance for reading ISR
    Servo* Servo::instances[Servo::MAX_INSTANCES];
    
    Servo* Servo::create(ServoModel servoModel){
        if(instanceCount >= MAX_INSTANCES || TIMER_NUMBER <= 0){
            return NULL;
        } else {
            if(instanceCount == 0){
                for (byte i = 0; i < MAX_INSTANCES; i++) {
                    instances[i] = NULL;
                }

                cli();
                timer = ISRTimer::create(TIMER_NUMBER, TIMER_PRESCALER, ISRTimer::TimerMode::COMPARE);
                timer->registerCallback(&Servo::handleTimerInterrupt, ISRTimer::HIGHEST_PRIORITY);
                sei();
                if (timer == NULL)return NULL;
            }

            //get first available slot
            byte idx = 0;
            for (byte i = 0; i < MAX_INSTANCES; i++) {
                if (instances[i] == NULL) {
                    idx = i;
                    break;
                }
            }

            //create the instance, set the index for future ref and update the count
            Servo* servo = new Servo(servoModel);
            instances[idx] = servo;
            instanceCount++;
            return servo;
        }
    }

    void Servo::destroy(Servo* servo) {
        for (int i = 0; i < MAX_INSTANCES; i++) {
            if (instances[i] == servo) {
                if (servo->attached()) {
                    servo->detach();
                }

                delete instances[i];
                instances[i] = NULL;
                instanceCount--;
            }
        }
    }

    void Servo::updateTimerCompareValue() {
        if (instanceCount == 0)return;

        Servo* servo = instances[0];
        unsigned int m = servo->resolution * servo->getMicrosForOneDegree();
        for (byte i = 1; i < MAX_INSTANCES; i++) {
            servo = instances[i];
            m = ISRTimer::gcd(m, servo->resolution * servo->getMicrosForOneDegree());
        }
        unsigned int comp = timer->microsToTicks(m);
        timer->setCompareValue(&Servo::handleTimerInterrupt, comp);

        return servo;
    }

    void Servo::handleTimerInterrupt(){
        static Servo* servo = instances[currentInstance];

        servo->onTimerInterrupt();
    }

    //Creation
    Servo::Servo(ServoModel model){
        this->model = model;


        switch(this->model){
            case ServoModel::MG996:
                min = 600; //0
                max = 2400; //180 deg (the diff between max and min should be multiples of the range of motion)
                pwmDuration = 20000; //in microsends (used to set the low duration of the puse)
                rangeOfMotion = 180;
                break;
        }

        //work out timer requirements for 1 degree of motion
        //we don't go finer than that .. em.g MG996 is 1 deg = 10 micros
        microsForOneDegree = (max - min) / rangeOfMotion;
    }
  
    Servo::~Servo() {
        if (attached()) {
            detach();
        }
    }

    
    unsigned int Servo::getMicrosForOneDegree() {
        return microsForOneDegree;
    }

    void Servo::attach(byte pinNumber){
        servoPin = pinNumber;
        attachedToPin = true;
        if (!timer->isEnabled()) {
            updateTimerCompareValue();
            timer->enable();
        }
        servoPulse = LOW;
        pinMode(servoPin, OUTPUT);
        digitalWrite(servoPin, servoPulse);
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

    void Servo::setResolution(unsigned int resolution) {
        this->resolution = resolution;
    }

    void Servo::onTimerInterrupt() {

        if (servoPulse == LOW && interruptCount >= pulseLowInInterrupts) {
            //here we want to go high
            servoPulse = HIGH;
            digitalWrite(servoPin, servoPulse);
            //PORTH = PORTH | 0b00010000;
            interruptCount = 0;
            
        }
        else if (servoPulse == HIGH && interruptCount >= pulseHighInInterrupts) {
            servoPulse = LOW;
            //PORTH = PORTH & 0b11101111;
            digitalWrite(servoPin, servoPulse);
            interruptCount = 0;
        }
        else {
            interruptCount++;
        }
    }

    void Servo::writeMicroseconds(unsigned long microseconds){
        if(!attached())return;

        microseconds += trimFactor;

        if(microseconds < min)microseconds = min;
        if(microseconds > max)microseconds = max;

        //Serial.print("Attemption to write microseconds: "); Serial.println(microseconds);

        pulseHighInInterrupts = timer->microsToInterrupts(&Servo::handleTimerInterrupt, microseconds);
        pulseLowInInterrupts = timer->microsToInterrupts(&Servo::handleTimerInterrupt, pwmDuration - microseconds);

        //Serial.print("Setting pulse high in interrupts to: "); Serial.println(pulseHighInInterrupts);
        //Serial.print("Setting pulse low in interrupts to: "); Serial.println(pulseLowInInterrupts);
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
            moveDuration = microsForOneDegree * (unsigned int)abs(position - pos);
        }
        unsigned long microseconds = map(pos, 0, 180, min, max);
        writeMicroseconds(microseconds);
        position = pos;
        return position;
    }

    int Servo::read(){
        if(isMoving()){
            return position - direction*((moveDuration - (micros() - startedMoving)) / microsForOneDegree);
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

    unsigned long Servo::getPulseDuration() {
        //return pulseDuration;
        return measuredPulseDuration;
    }
} //end namespace
