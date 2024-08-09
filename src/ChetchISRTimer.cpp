#include "ChetchISRTimer.h"

namespace Chetch{
#if defined(USE_ISR_TIMER_NUMBER3)
    ISR(TIMER3_COMPA_vect) {
        //we pass the index (not the number) for sake of speed
        //ISRTimer::timers[2]->callbacks[0](); // onTimerInterrupt();
        ISRTimer::timers[2]->onTimerInterrupt();
    }
#endif
#if defined(USE_ISR_TIMER_NUMBER4)
    ISR(TIMER4_COMPA_vect) {
        //we pass the index (not the number) for sake of speed
        ISRTimer::timers[3]->onTimerInterrupt();
    }
#endif


    //static stuff
    //byte ISRTimer::timerIndex = 0;
    byte ISRTimer::timerCount = 0;
    ISRTimer* ISRTimer::timers[ISRTimer::MAX_TIMERS];

    ISRTimer *ISRTimer::create(byte timerNumber, uint16_t prescaler, TimerMode mode){
        if (timerNumber < 1)timerNumber = 1;
        if (timerNumber > MAX_TIMERS)timerNumber = MAX_TIMERS;

        //make clean on first use
        if (timerCount == 0) {
            for (byte i = 0; i < MAX_TIMERS; i++) {
                timers[i] = NULL;
            }
        }

        byte timerIndex = timerNumber - 1;
        if (timers[timerIndex] == NULL) {
            timers[timerIndex] = new ISRTimer(timerNumber, prescaler, mode);
            timerCount++;
        }
        return timers[timerIndex];
    }

    ISRTimer* ISRTimer::getTimer(byte timerNumber) {
        if (timerNumber < 1)timerNumber = 1;
        if (timerNumber > MAX_TIMERS)timerNumber = MAX_TIMERS;

        byte timerIndex = timerNumber - 1;
        return timers[timerIndex];
    }

    void ISRTimer::handleTimerInterrupt(byte timerIndex) {
        timers[timerIndex]->onTimerInterrupt();
    }

    uint16_t ISRTimer::gcd(uint16_t a, uint16_t b) {
        uint16_t temp;
        while (b != 0)
        {
            temp = a % b;
            a = b;
            b = temp;
        }
        return a;
    }
  
    ISRTimer::ISRTimer(byte timerNumber, uint16_t prescaler, TimerMode mode){
        this->timerNumber = timerNumber;
        this->timerMode = mode;
        this->prescaler = prescaler;

        for (int i = 0; i < MAX_CALLBACKS; i++) {
            callbacks[i] = NULL;
            interruptCounts[i] = 0;
        }

        bool validTimer = true;
        switch(timerNumber){
#if defined(ARDUINO_AVR_MEGA2560)
            case 3:
                TCCR3A = 0; // set entire TCCRnA register to 0
                TCCR3B = 0; // same for TCCRnB
                TCNT3  = 0; //initialize counter value to 0
          
                //Turn on CTC mode
                //TODO: vary this depending on mode
                TCCR3B |= (1 << WGM32);
          
                //Set prescaler
                //TODO: vary this depending on prescaler value
                TCCR3B |= (1 << CS31); 

                //keep pointers so that we can generalise functionality
                this->TIMSKn = &TIMSK3;
                this->TCNTn = &TCNT3;
                this->OCRnA = &OCR3A;
                this->TIFRn = &TIFR3;
                this->enableBitPosition = OCIE3A;
                this->pendingISRBitPosition = OCF3A;
                break;

            case 4:
                TCCR4A = 0; // set entire TCCRnA register to 0
                TCCR4B = 0; // same for TCCRnB
                TCNT4  = 0; //initialize counter value to 0
          
                //Turn on CTC mode
                //TODO: vary this depending on mode
                TCCR4B |= (1 << WGM42);
          
                //Set prescaler
                //TODO: vary this depending on prescaler value
                TCCR4B |= (1 << CS41); 

                //keep pointers so that we can generalise functionality
                this->TIMSKn = &TIMSK4;
                this->TCNTn = &TCNT4;
                this->OCRnA = &OCR4A;
                this->TIFRn = &TIFR4;
                this->enableBitPosition = OCIE4A;
                this->pendingISRBitPosition = OCF4A;
                break;
#endif
             default:
                validTimer = false;
                break;
        }

        if(validTimer)disable();
    }
  
    bool ISRTimer::registerCallback(ISRTimerCallback callback, int priority, uint16_t comp) {
        int idx = -1;
        switch (priority) {
        case LOWEST_PRIORITY:
            idx = MAX_CALLBACKS - 1;
            break;

        case HIGHEST_PRIORITY:
            idx = 0;
            break;

        default:
            idx = priority - 1;
            break;
        }
        
        //check the index is meaningful
        if (idx < 0 || idx >= MAX_CALLBACKS) {
            return false;
        }

        //look for an available slot
        if (callbacks[idx] != NULL) {
            return false;
        }
        else {
            //is this already registered?
            for (int i = 0; i < MAX_CALLBACKS; i++) {
                if (callbacks[i] == callback) {
                    return false;
                }
            }
            
            //here is a fresh registration
            callbacks[idx] = callback;
            callbackCount++;
            if (callbackCount == 1) {
                singleCallback = callback;
            }
            else {
                singleCallback = NULL;
            }
            if (comp > 0) {
                return setCompareValue(callback, comp) != 0;
            }
            else {
                return true;
            }
        }
    }

    int ISRTimer::getCallbackIdx(ISRTimerCallback callback) {
        int idx = -1;
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (callbacks[i] == callback) {
                idx = i;
                break;
            }
        }
        return idx;
    }

    uint16_t ISRTimer::setCompareValue(ISRTimerCallback callback, uint16_t comp) {
        //find the index of the callback
        int idx = getCallbackIdx(callback);
        if (idx == -1)return 0;
        
        //now work out the new compare value
        uint16_t currentComp = getCompareA();
        uint16_t newComparison = comp;
        if (currentComp != 0) {
            for (int i = 0; i < MAX_CALLBACKS; i++) {
                if (interruptCounts[i] > 0 && i != idx) {
                    newComparison = gcd(newComparison, currentComp * interruptCounts[i]);
                }
            }
        }

        //set the compare value and set this particular interrupt count
        setCompareA(newComparison);
        interruptCounts[idx] = comp / newComparison;
        maxInterruptCount = interruptCounts[idx];

        //adjust other interrupt counts
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (i != idx) {
                interruptCounts[i] = (interruptCounts[i] * currentComp) / newComparison;
                if (interruptCounts[i] > maxInterruptCount)maxInterruptCount = interruptCounts[i];
            }
        }

        return maxInterruptCount;
    }

    void ISRTimer::onTimerInterrupt() {
        static uint16_t interruptCount = 0;
        static uint16_t counters[MAX_CALLBACKS];

        if (singleCallback != NULL) {
            singleCallback();
            return;
        }

        //unsigned long started = micros();
        if (interruptCount == maxInterruptCount) {
            interruptCount = 0;
            counters[0] = interruptCounts[0];
            counters[1] = interruptCounts[1];
            //counters[2] = interruptCounts[2];
            //counters[3] = interruptCounts[3];
        }

        interruptCount++;
        
        if (callbacks[0] != NULL) {
            if (interruptCounts[0] == 1) {
                callbacks[0]();
            }
            else if(counters[0] == interruptCount) {
                counters[0] += interruptCounts[0];
                callbacks[0]();
            }
        }
        if (callbacks[1] != NULL) {
            if (interruptCounts[1] == 1) {
                callbacks[1]();
            }
            else if (counters[1] == interruptCount) {
                counters[1] += interruptCounts[1];
                callbacks[1]();
            }
        }
        
        //interruptDuration = micros() - started;
        //if(interruptDuration > maxInterruptDuration)maxInterruptDuration = interruptDuration;
    }

    void ISRTimer::enable(){
        enabled = true;
        *TIMSKn |= (1 << enableBitPosition);
    }
    
    void ISRTimer::disable(){
        *TIMSKn &= ~(1 << enableBitPosition);   
        enabled = false;
    }

    bool ISRTimer::isEnabled(){
        return enabled;
    }
  
    void ISRTimer::setCompareA(uint16_t cnt, uint16_t comp){
        *TCNTn = cnt; //set counter
        *OCRnA  = comp; //set compare
    }

    void ISRTimer::setCompareA(uint16_t comp){
        *OCRnA  = comp; //set compare
    }

    uint16_t ISRTimer::getCompareA(){
        return *OCRnA;
    }

    uint32_t ISRTimer::microsToTicks(uint32_t microseconds){
        return (microseconds * 16) / prescaler;
    }

    uint32_t ISRTimer::ticksToMicros(uint32_t ticks){
        return (ticks * prescaler) / 16;
    }

    uint32_t ISRTimer::microsToInterrupts(ISRTimerCallback callback, uint32_t microseconds) {
        int idx = getCallbackIdx(callback);
        if (idx == -1)return 0;

        uint16_t comp = interruptCounts[idx] * getCompareA();
        if (comp == 0) {
            return;
        }
        else {
            uint16_t ticks = microsToTicks(microseconds);
            return ticks / comp;
        }   
    }

    uint32_t ISRTimer::interruptsToMicros(ISRTimerCallback callback, uint32_t interrupts) {
        int idx = getCallbackIdx(callback);
        if (idx == -1)return 0;

        uint16_t comp = interruptCounts[idx] * getCompareA();
        return ticksToMicros(comp * interrupts);
    }
} //end namespace
