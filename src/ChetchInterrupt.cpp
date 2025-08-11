#include "ChetchInterrupt.h"
#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

namespace Chetch{

    byte CInterrupt::pinCount = 0;
    CInterrupt::Callback CInterrupt::callbacks[];

    bool CInterrupt::isSupportedPin(uint8_t pinNumber)
    {
 #ifdef ARDUINO_AVR_MEGA2560
        if (pinNumber == 2 || pinNumber == 3 || (pinNumber >= 10 && pinNumber <= 15) || (pinNumber >= 18 && pinNumber <= 21)){            
            return true;
        } else {
            return false;
        }
#elif defined(ARDUINO_AVR_UNO)
        if (pinNumber == 2 || pinNumber == 3){            
            return true;
        } else {
            return false;
        }
#elif defined(ARDUINO_AVR_NANO)
        if (pinNumber == 2 || pinNumber == 3){            
            return true;
        } else {
            return false;
        }
#else
        return false;
#endif
    }

    void CInterrupt::handleInterrupt()
    {
        if (arduinoInterruptedPin == callbacks[0].pin) {
            callbacks[0].onInterrupt(arduinoInterruptedPin, callbacks[0].tag);
        } else if (arduinoInterruptedPin == callbacks[1].pin) {
            callbacks[1].onInterrupt(arduinoInterruptedPin, callbacks[1].tag);
        } else if (arduinoInterruptedPin == callbacks[2].pin) {
            callbacks[2].onInterrupt(arduinoInterruptedPin, callbacks[2].tag);
        } else if (arduinoInterruptedPin == callbacks[3].pin) {
            callbacks[3].onInterrupt(arduinoInterruptedPin, callbacks[3].tag);
        } //else if.... add more here if required ... don't forget to increase MAX_PINS
        
    }

    bool CInterrupt::addInterruptListener(uint8_t pinNumber, uint8_t tag, InterruptListener listener, uint8_t mode) {
        if (pinCount >= MAX_PINS || !isSupportedPin(pinNumber))return false;

        for (byte i = 0; i < MAX_PINS; i++) {
            if (callbacks[i].pin == pinNumber)return false;

            if (callbacks[i].pin == 0) {
                callbacks[i].pin = pinNumber;
                callbacks[i].tag = tag;
                callbacks[i].onInterrupt = listener;
                pinCount++;
                break;
            }
        }

        enableInterrupt(pinNumber, handleInterrupt, mode);
        return true;
    }


    bool CInterrupt::removeInterruptListener(uint8_t pinNumber) {
        bool found = false;
        for (byte i = 0; i < MAX_PINS; i++) {
            if (callbacks[i].pin == pinNumber) {
                callbacks[i].pin = 0;
                callbacks[i].onInterrupt = NULL;
                pinCount--;
                found = true;
                break;
            }
        }
        if (found) {
            disableInterrupt(pinNumber);
            return true;
        } else {
            return false;
        }
    }
} //end namespace
