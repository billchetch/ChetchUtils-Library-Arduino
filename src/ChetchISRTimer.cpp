#include "ChetchISRTimer.h"

namespace Chetch{
/*
PRESCALER INFO
Use CSNn constants where N = Timer number and n is bit position (2, 1, 0).
e.g. Timer 1 has bit position constants :  CS12, CS11, CS10
000 : no clock source TC1 is OFF
1 << CS10 = 001 : divide by 1 (if 16Mhz chip then 16 times every micro second)
1 << CS11 = 010 : divide by 8 (if 16Mhz chip then 2 times every micro second)
1 << CS11 | 1 << CS10 = 011 : divide by 64 (if 16Mhz chip then every 16 microseconds)
1 << CS12 = 100 : divide by 256 (if 16Mhz chip then every 64 microseconds)
1 << CS12 | 1 << CS10 = 101 : divide by 1024 (if 16Mhz chip then 256 microseconds)
*/

#if defined(USE_ISR_TIMER_NUMBER1)
    ISR(TIMER1_COMPA_vect) {
        ISRTimer::timers[0]->onTimerInterrupt();
    }
#endif
#if defined(USE_ISR_TIMER_NUMBER2)
    ISR(TIMER2_COMPA_vect) {
        ISRTimer::timers[1]->onTimerInterrupt();
    }
#endif
#if defined(USE_ISR_TIMER_NUMBER3)
    ISR(TIMER3_COMPA_vect) {
        ISRTimer::timers[2]->onTimerInterrupt();
    }
#endif
#if defined(USE_ISR_TIMER_NUMBER4)
    ISR(TIMER4_COMPA_vect) {
        ISRTimer::timers[3]->onTimerInterrupt();
    }
#endif


    //static stuff
    //byte ISRTimer::timerIndex = 0;
    byte ISRTimer::timerCount = 0;
    ISRTimer* ISRTimer::timers[ISRTimer::MAx_INSTANCES];

    ISRTimer *ISRTimer::create(byte timerNumber, uint16_t prescaler, TimerMode mode){
        if (timerNumber < 1)timerNumber = 1;
        if (timerNumber > MAx_INSTANCES)timerNumber = MAx_INSTANCES;

        //make clean on first use
        if (timerCount == 0) {
            for (byte i = 0; i < MAx_INSTANCES; i++) {
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
        if (timerNumber > MAx_INSTANCES)timerNumber = MAx_INSTANCES;

        byte timerIndex = timerNumber - 1;
        return timers[timerIndex];
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
            callbacks[i].onTimer = NULL;
            interruptCounts[i] = 0;
        }

        bool validTimer = true;
        uint16_t prescale = 0;
#if defined(ARDUINO_AVR_MEGA2560)
        
        switch(timerNumber){
            case 3:
                TCCR3A = 0; // set entire TCCRnA register to 0
                TCCR3B = 0; // same for TCCRnB
                TCNT3  = 0; //initialize counter value to 0
          
                //Turn on CTC mode
                //TODO: vary this depending on mode
                TCCR3B |= (1 << WGM32);
          
                //Set prescaler
                //TODO: vary this depending on prescaler value
                TCCR3B |= prescale; 

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
                TCCR4B |= prescaler; 

                //keep pointers so that we can generalise functionality
                this->TIMSKn = &TIMSK4;
                this->TCNTn = &TCNT4;
                this->OCRnA = &OCR4A;
                this->TIFRn = &TIFR4;
                this->enableBitPosition = OCIE4A;
                this->pendingISRBitPosition = OCF4A;
                break;

            default:
                validTimer = false;
                break;
        }
#elif defined(ARDUINO_AVR_NANO)
        switch(prescaler){
            case 0: prescale = 0; break;
            case 1: prescale = 1 << CS10; break;
            case 8: prescale = 1 << CS11; break;
            case 64: prescale = (1 << CS11 | 1 << CS10); break;
            case 256: prescale = 1 << CS12; break;
            case 1024: prescale = (1 << CS12 | 1 << CS10); break;
            default:
                prescale = 0;
                break;
        }
        
        switch(timerNumber){
            case 1:
                //TCNT1, OCR1A, OCR1B, TCCR1A, TCCR1B, and TIMSK1
                TCCR1A = 0; // set entire TCCRnA register to 0
                TCCR1B = 0; // same for TCCRnB
                TCNT1  = 0; //initialize counter value to 0
          
                //Turn on CTC mode
                //TODO: vary this depending on mode
                TCCR1B |= (1 << WGM12);
          
                //Set prescaler
                //TODO: vary this depending on prescaler value
                TCCR1B |= prescale; 

                //keep pointers so that we can generalise functionality
                this->TIMSKn = &TIMSK1;
                this->TCNTn = &TCNT1;
                this->OCRnA = &OCR1A;
                this->TIFRn = &TIFR1;
                this->enableBitPosition = OCIE1A;
                this->pendingISRBitPosition = OCF1A;
                break;

            default:
                validTimer = false;
                break;
        }
#endif

        if(validTimer)disable();
    }
  
    bool ISRTimer::addListener(uint8_t id, TimerListener listener, int priority, uint16_t comp) {
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

        //check we even have a listener
        if (listener == NULL) {
            return false;
        }

        //ID must be greater than 0
        if(id == 0){
            return false;
        }

        //look for an available slot
        if (callbacks[idx].onTimer != NULL) {
            return false;
        }
        else {
            //is this already registered?
            for (int i = 0; i < MAX_CALLBACKS; i++) {
                if (callbacks[i].id == id && callbacks[i].onTimer != NULL) {
                    return false;
                }
            }
            
            //here is a fresh registration
            /*Serial.print("Adding listener with ID: ");
            Serial.println(id);
            Serial.print("To callback at index: ");
            Serial.println(idx);
            Serial.print("And set compare value to: ");
            Serial.println(comp);*/
            callbacks[idx].id = id;
            callbacks[idx].onTimer = listener;
            callbackCount++;

            if(callbackCount == 1){
                singleCallback = &callbacks[idx];
            } else {
                singleCallback = NULL;
            }
            
            if (comp > 0) {
                return setCompareValue(id, comp) != 0;
            }
            else {
                return true;
            }
        }
    }

    bool ISRTimer::removeListener(uint8_t id){
        int idx = getCallbackIdx(id);
        if(idx >= 0){
            callbacks[idx].id = 0;
            callbacks[idx].onTimer = NULL;
        }
    }

    int ISRTimer::getCallbackIdx(uint8_t id) {
        int idx = -1;
        for (int i = 0; i < MAX_CALLBACKS; i++) {
            if (callbacks[i].id == id) {
                idx = i;
                break;
            }
        }
        return idx;
    }

    uint16_t ISRTimer::setCompareValue(uint8_t id, uint16_t comp) {
        //find the index of the callback
        int idx = getCallbackIdx(id);
        if (idx == -1){
            return 0;
        }
        
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

        if (callbackCount == 0)return;

        if (singleCallback != NULL) {
            singleCallback->onTimer(singleCallback->id);
            return;
        }

        /*
        //unsigned long started = micros();
        if (interruptCount == maxInterruptCount) {
            interruptCount = 0;
            counters[0] = interruptCounts[0];
            counters[1] = interruptCounts[1];
            //counters[2] = interruptCounts[2];
            //counters[3] = interruptCounts[3];
        }

        interruptCount++;
        
        if (callbacks[0].onTimer != NULL) {
            if (interruptCounts[0] == 1) {
                callbacks[0].onTimer((callbacks[0].id));
            }
            else if(counters[0] == interruptCount) {
                counters[0] += interruptCounts[0];
                callbacks[0].onTimer((callbacks[0].id));
            }
        }
        if (callbacks[1].onTimer != NULL) {
            if (interruptCounts[1] == 1) {
                callbacks[1].onTimer((callbacks[1].id));
            }
            else if (counters[1] == interruptCount) {
                counters[1] += interruptCounts[1];
                callbacks[1].onTimer((callbacks[1].id));
            }
        }*/
        
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

    uint32_t ISRTimer::microsToInterrupts(uint8_t id, uint32_t microseconds) {
        int idx = getCallbackIdx(id);
        if (idx == -1)return 0;

        uint16_t comp = interruptCounts[idx] * getCompareA();
        if (comp == 0) {
            return 0;
        }
        else {
            uint16_t ticks = microsToTicks(microseconds);
            return ticks / comp;
        }   
    }

    uint32_t ISRTimer::interruptsToMicros(uint8_t id, uint32_t interrupts) {
        int idx = getCallbackIdx(id);
        if (idx == -1)return 0;

        uint32_t comp = interruptCounts[idx] * getCompareA();
        return ticksToMicros(comp * interrupts);
    }
} //end namespace
