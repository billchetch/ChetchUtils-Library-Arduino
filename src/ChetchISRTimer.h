#ifndef CHETCH_ISR_TIMER_H
#define CHETCH_ISR_TIMER_H

#include <Arduino.h>

#if defined(ARDUINO_AVR_MEGA2560)
    #define USE_ISR_TIMER_NUMBER3
    #define USE_ISR_TIMER_NUMBER4
#else

#endif

namespace Chetch{
    
    class ISRTimer{

		public:
			enum class TimerMode{
              NOT_SET,
              COMPARE,
            };

            static const int LOWEST_PRIORITY = -1;
            static const int HIGHEST_PRIORITY = -2;

            static const byte MAX_TIMERS = 6; //this should be defined
            static const byte MAX_CALLBACKS = 2; //maximum number per timer

            static ISRTimer* timers[];
            static ISRTimer* getTimer(byte timerNumber);
            static byte timerCount;
    
            //locals
            byte timerNumber;
            //byte priority;
            TimerMode timerMode;
            uint16_t prescaler;
            uint16_t enableBitPosition; //enable/disable bit
            uint16_t pendingISRBitPosition; //flag bit to show if timer has pending ISR
            volatile uint8_t *TIMSKn;
            volatile uint16_t *TCNTn;
            volatile uint16_t *OCRnA;
            volatile uint8_t *TIFRn;
            volatile bool enabled = false;	
	
            typedef void (*ISRTimerCallback)();
            
            ISRTimerCallback callbacks[MAX_CALLBACKS];
            ISRTimerCallback singleCallback = NULL; //if only one is registered this is used in the ISR function for speed
            uint16_t interruptCounts[MAX_CALLBACKS]; //how many times the interrupt is called before calling the callback
            uint16_t maxInterruptCount = 0; //calculated from comparison counts .. 
            byte callbackCount = 0;

            volatile unsigned long interruptDuration = 0;
            volatile unsigned long maxInterruptDuration = 0;

        private:
            int getCallbackIdx(ISRTimerCallback callback);

		public:
            static ISRTimer *create(byte timerNumber, uint16_t prescaler, TimerMode mode = ISRTimer::TimerMode::COMPARE);
            static void handleTimerInterrupt(byte timerIndex);
            static uint16_t gcd(uint16_t a, uint16_t b);

            //static inline bool freeToExecute(byte priority, uint16_t executionEnd);

            ISRTimer(byte timerNumber, uint16_t prescaler, TimerMode mode);

            bool registerCallback(ISRTimerCallback callback, int priority, uint16_t comp = 0);
            uint16_t setCompareValue(ISRTimerCallback callback, uint16_t comp);

            void onTimerInterrupt();

            bool isEnabled();
			void setCompareA(uint16_t cnt, uint16_t comp);
            void setCompareA(uint16_t comp);
            uint16_t getCompareA();
			void enable();
			void disable();
            
            uint32_t microsToTicks(uint32_t microseconds); //how many ticks of the timer
            uint32_t ticksToMicros(uint32_t ticks); //how many micros for a certin number of ticks
            uint32_t microsToInterrupts(ISRTimerCallback callback, uint32_t microseconds); //how many interrupts are called in a given period of micros
            uint32_t interruptsToMicros(ISRTimerCallback callback, uint32_t interrupts); //how many micros are a certain number of interrupts
    }; //end class
} //end namespae
#endif