#ifndef CHETCH_ISR_TIMER_H
#define CHETCH_ISR_TIMER_H

#include <Arduino.h>

namespace Chetch{
    
    class ISRTimer{
		public:
			enum class TimerMode{
              NOT_SET,
              COMPARE,
            };

            static const byte MAX_TIMERS = 3;
            static ISRTimer* timers[];
            static byte timerIndex;
    
            byte timerNumber;
            byte priority;
            TimerMode timerMode;
            uint16_t prescaler;
            uint16_t enableBitPosition; //enable/disable bit
            uint16_t pendingISRBitPosition; //flag bit to show if timer has pending ISR
            volatile uint8_t *TIMSKn;
            volatile uint16_t *TCNTn;
            volatile uint16_t *OCRnA;
            volatile uint8_t *TIFRn;
            volatile bool enabled = false;	
	
		public:
            static ISRTimer *create(byte timerNumber, uint16_t prescaler, TimerMode mode = ISRTimer::TimerMode::COMPARE);
            static inline bool freeToExecute(byte priority, uint16_t executionEnd);

            ISRTimer(byte timerNumber, TimerMode mode, uint16_t prescaler);

            bool isEnabled();
			void setCompareA(uint16_t cnt, uint16_t comp);
            void setCompareA(uint16_t comp);
            uint16_t getCompareA();
			void enable();
			void disable();
            bool freeToExecute(uint16_t executionEnd);
            uint16_t microsToTicks(uint32_t microseconds);
            uint16_t ticksToMicros(uint32_t ticks);
    }; //end class
} //end namespae
#endif