#ifndef CHETCH_SERVO_H
#define CHETCH_SERVO_H

#include <Arduino.h>
#include <ChetchISRTimer.h>

namespace Chetch{
    
    class Servo{
		public:
			enum class ServoModel{
              MG996,
            };

            static const byte MAX_INSTANCES = 1; //IMPORTANT!:Class requires modification if you want to increase instances beyond 1

         public: //TODO: make private where necessary (some things should remain public so they are quick to access in the ISR)
            static ISRTimer* timer;
            static byte instanceCount;
            static byte currentInstance; //each time an ISR is fired this updates so as to read the next instance voltage
            static Servo* instances[];
            
            byte instanceIndex = 0;

            //servo characteristics (may vary based on model)
            ServoModel model;
            unsigned int min = 1000; //in micros
            unsigned int max = 2000;
            unsigned long pwmDuration = 0; //also thought of as frequency of servo
            unsigned int rangeOfMotion = 180; //standard
            unsigned int microsForOneDegree = 0; //in microseconds to move one degree 

            //setup
            bool attachedToPin = false;
            byte servoPin = 0;
            int trimFactor = 0; //in degrees (use this to trim the servo)

            //operation
            int position = -1;  //negative indicaates not yet known/assigned
            volatile bool servoPulse = LOW;

            unsigned int interruptCount = 0;
            unsigned int pulseHighInInterrupts = 0; //in timer ticks, High + Low should be the pwm duration in ticks
            unsigned int pulseLowInInterrupts = 0;
            unsigned long startedMoving = 0;
            unsigned long moveDuration = 0;
            int direction = 0; //1 = anti-clockwise, -1 = clockwise
	
            //debug stuff
            volatile unsigned long measuredPulseDuration = 0;
            volatile unsigned long startedTimingOn = 0;


		public:
            static Servo* create(ServoModel servoModel);
            static void handleTimerInterrupt();
        
            Servo(ServoModel servoModel);
            ~Servo();

        private:
            void onTimerInterrupt();
            void writeMicroseconds(unsigned long microseconds);
            
        public:
            //creation
            void setInstanceIndex(byte idx);
            unsigned int getMicrosForOneDegree();

            //setup
            void attach(byte pinNumber);
            bool attached();
            void detach();
            void setTrim(int trimFactor, bool usingDegrees = true);

            //operation
            int write(int pos); //in degrees (0 to 180)
            
            
            //IMPORTANT: read or ismoving must be called regularly (i.e. before micros() overflows)) to avoid the rare condition 
            //where the micros() function overflows and the logic for isMoving gives a false positive
            int read();
            bool isMoving();

            //reporting
            unsigned long getPulseDuration();
    }; //end class
} //end namespae
#endif