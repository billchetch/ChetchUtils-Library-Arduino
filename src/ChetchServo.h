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
            static byte instanceIndex;
            static byte currentInstance; //each time an ISR is fired this updates so as to read the next instance voltage
            static Servo* instances[];
            
            ServoModel model;
            bool servoPulse = LOW;
            bool attachedToPin = false;
            byte servoPin = 0;
            unsigned int min = 1000;
            unsigned int max = 2000;
            int trimFactor = 0;
            unsigned long rotationSpeed = 0; //in microseconds to move one degree 
            int position = -1;  //negative indicaates not yet known/assigned

            unsigned int pulseEndInTicks = 0; //in timer ticks
            unsigned int pulseStartInTicks = 0;
            unsigned long startedMoving = 0;
            unsigned long moveDuration = 0;
            int direction = 0; //1 = anti-clockwise, -1 = clockwise
	
		public:
            
            static Servo* create(ServoModel servoModel);
            static void handleTimerInterrupt();
        
        private:
            Servo(ServoModel servoModel);
            void writeMicroseconds(unsigned long microseconds);
            
        public:
            void attach(byte pinNumber);
            bool attached();
            void detach();
            void setTrim(int trimFactor, bool usingDegrees = true);
            int write(int pos); //in degrees (0 to 180)

            //IMPORTANT: read or ismoving must be called regularly (i.e. before micros() overflows)) to avoid the rare condition 
            //where the micros() function overflows and the logic for isMoving gives a false positive
            int read();
            bool isMoving();
    }; //end class
} //end namespae
#endif