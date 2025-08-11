#ifndef CHETCH_INTERRUPT_H
#define CHETCH_INTERRUPT_H

#include <Arduino.h>

namespace Chetch{
    
    class CInterrupt {
        private:
            typedef void (*InterruptListener)(uint8_t p, uint8_t t);

            struct Callback {
                uint8_t pin = 0;
                uint8_t tag = 0;
                InterruptListener onInterrupt = NULL;
            };

            static const byte MAX_PINS = 4;
            static Callback callbacks[MAX_PINS];
            static byte pinCount;

        private:
            static bool isSupportedPin(uint8_t pinNumber);
            static void handleInterrupt();

        public:
            static bool addInterruptListener(uint8_t pinNumber, uint8_t tag, InterruptListener listener, uint8_t mode);
            static bool removeInterruptListener(uint8_t pinNumber);
    }; //end class
} //end namespae
#endif