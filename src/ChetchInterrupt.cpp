#include "ChetchInterrupt.h"
#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

namespace Chetch{

    byte CInterrupt::pinCount = 0;

    bool CInterrupt::isSupportedPin(uint8_t pinNumber)
    {
        if (pinNumber > 2) {
            return true;
        }
        else {
            return false;
        }
    }

    void CInterrupt::handleInterrupt()
    {
        if (arduinoInterruptedPin == callbacks[0].pin) {
            callbacks[0].onInterrupt(arduinoInterruptedPin);
        } else if (arduinoInterruptedPin == callbacks[1].pin) {
            callbacks[1].onInterrupt(arduinoInterruptedPin);
        } else if (arduinoInterruptedPin == callbacks[2].pin) {
            callbacks[2].onInterrupt(arduinoInterruptedPin);
        } else if (arduinoInterruptedPin == callbacks[3].pin) {
            callbacks[3].onInterrupt(arduinoInterruptedPin);
        }
    }

    bool CInterrupt::addInterrupt(uint8_t pinNumber, void (*onInterrupt)(uint8_t pin), uint8_t mode) {
        if (pinCount >= MAX_PINS || !isSupportedPin(pinNumber))return false;

        for (byte i = 0; i < MAX_PINS; i++) {
            if (callbacks[i].pin == pinNumber)return false;

            if (callbacks[i].pin == 0) {
                callbacks[i].pin = pinNumber;
                callbacks[i].onInterrupt = onInterrupt;
                pinCount++;
                break;
            }
        }

        enableInterrupt(pinNumber, handleInterrupt, mode);
        return true;
    }

    bool CInterrupt::removeInterrupt(uint8_t pinNumber) {
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
