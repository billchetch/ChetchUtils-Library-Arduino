#include "ChetchISRTimer.h"

namespace Chetch{
    
    byte ISRTimer::timerIndex = 0;
    ISRTimer* ISRTimer::timers[ISRTimer::MAX_TIMERS];

    static ISRTimer *ISRTimer::create(byte timerNumber, uint16_t prescaler, TimerMode mode = ISRTimer::TimerMode::COMPARE){
        for(byte i = 0; i < timerIndex; i++){
            if(timers[i]->timerNumber == timerNumber){
                return timers[i];
            }
        }
      
        if(timerNumber == 3 || timerNumber == 4){
            ISRTimer *timer = new ISRTimer(timerNumber, mode, prescaler);
      
            timers[timerIndex++] = timer;
            timer->priority = timerIndex;
            return timer;
        } else {
            return NULL;
        }
    }

    //executionEnd is in microseconds ... checks if there is enough time before a timer with a higher priority
    //will fire
    static bool ISRTimer::freeToExecute(byte priority, uint16_t executionEnd){
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
    }
  
    ISRTimer::ISRTimer(byte timerNumber, TimerMode mode, uint16_t prescaler){
        this->timerNumber = timerNumber;
        this->timerMode = mode;
        this->prescaler = prescaler;

        bool validTimer = true;
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

             default:
                validTimer = false;
                break;
        }

        if(validTimer)disable();
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
    bool ISRTimer::freeToExecute(uint16_t executionEnd){
        return ISRTimer::freeToExecute(this->priority, executionEnd);
    }

    uint16_t ISRTimer::microsToTicks(uint32_t microseconds){
        return (microseconds * 16) / prescaler;
    }

    uint16_t ISRTimer::ticksToMicros(uint32_t ticks){
        return (ticks * prescaler) / 16;
    }
} //end namespace
