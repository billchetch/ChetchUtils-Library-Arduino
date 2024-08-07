#include "ChetchISRTimer.h"

namespace Chetch{
#if defined(USE_ISR_TIMER_NUMBER3)
    ISR(TIMER3_COMPA_vect) {
        //we pass the index (not the number) for sake of speed
        ISRTimer::handleTimerInterrupt(2);
    }
#endif
#if defined(USE_ISR_TIMER_NUMBER4)
    ISR(TIMER4_COMPA_vect) {
        //we pass the index (not the number) for sake of speed
        ISRTimer::handleTimerInterrupt(3);
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


    void ISRTimer::handleTimerInterrupt(byte timerIndex) {
        if (timers[timerIndex] != NULL) {
            timers[timerIndex]->onTimerInterrupt();
        }
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

    //executionEnd is in microseconds ... checks if there is enough time before a timer with a higher priority
    //will fire
    /*bool ISRTimer::freeToExecute(byte priority, uint16_t executionEnd) {
        if(timerIndex <= 0 || priority <= 1)return true;

        static ISRTimer *timer;
        
        uint16_t nextISR;
        timer = timers[0];
        if(timer->enabled){
            nextISR = ((*timer->OCRnA - *timer->TCNTn) * timer->prescaler) / 16;
            if(executionEnd >= nextISR)return false;  
            if(*timer->TIFRn & (1 << timer->pendingISRBitPosition))return false;   //pending ISR 
        }

        if(timerIndex <= 1 || priority <= 2)return true;
        timer = timers[1];
        if(timer->enabled){
            nextISR = ((*timer->OCRnA - *timer->TCNTn) * timer->prescaler) / 16;
            if(executionEnd >= nextISR)return false;  
            if(*timer->TIFRn & (1 << timer->pendingISRBitPosition))return false;   //pending ISR 
        }

        if(timerIndex <= 2 || priority <= 3)return true;
        timer = timers[2];
        if(timer->enabled){
            nextISR = ((*timer->OCRnA - *timer->TCNTn) * timer->prescaler) / 16;
            if(executionEnd >= nextISR)return false;  
            if(*timer->TIFRn & (1 << timer->pendingISRBitPosition))return false;   //pending ISR 
        }
  
        return true;
    }*/
    //end static stuff
  
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
  
    bool ISRTimer::registerCallback(ISRTimerCallback callback, byte priority, uint16_t comp) {
        int idx = priority - 1;
        if (callbacks[idx] != NULL) {
            return false;
        }
        else {
            //register callback
            for (int i = 0; i < MAX_CALLBACKS; i++) {
                if (callbacks[i] == callback) {
                    return false;
                }
            }
            
            callbacks[idx] = callback;
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
        static unsigned long interruptDuration = 0;
        static uint16_t interruptCount = 0;

        unsigned long started = micros();

        interruptCount++;

        if (callbacks[0] != NULL && interruptCount % interruptCounts[0] == 0) {
            callbacks[0]();
        }
        if (callbacks[1] != NULL && interruptCount % interruptCounts[1] == 0) {
            callbacks[1]();
        }
        if (callbacks[2] != NULL && interruptCount % interruptCounts[2] == 0) {
            callbacks[2]();
        }
        if (callbacks[3] != NULL && interruptCount % interruptCounts[3] == 0) {
            callbacks[3]();
        }
        
        if (interruptCount == maxInterruptCount) {
            interruptCount = 0;
        }

        interruptDuration = micros() - started;
        maxInterruptDuration = interruptDuration;
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

    //executionEnd is in microseconds
    /*bool ISRTimer::freeToExecute(uint16_t executionEnd) {
        return ISRTimer::freeToExecute(this->priority, executionEnd);
    }*/

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
