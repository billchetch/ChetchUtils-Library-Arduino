#ifndef CHETCH_ADC_H
#define CHETCH_ADC_H

#include <Arduino.h>

namespace Chetch{
    
    class CADC{
		public:
            enum class AnalogReference{
              AREF_DEFAULT,
              AREF_EXTERNAL,
              AREF_INTERNAL,
            };

        public:
            static void init(AnalogReference ref, bool triggerInterrupt);
            static void startRead(int analogPin);
			static bool isReading();
            static uint16_t readResult();
    }; //end class
} //end namespae
#endif