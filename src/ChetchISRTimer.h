#ifndef CHETCH_ISR_TIMER_H
#define CHETCH_ISR_TIMER_H

#include <Arduino.h>

#if defined(ARDUINO_AVR_MEGA2560)
    #define USE_ISR_TIMER_NUMBER3
    #define USE_ISR_TIMER_NUMBER4
    #define MAX_ISR_TIMERS 6
#elif defined(ARDUINO_AVR_NANO)
    #define USE_ISR_TIMER_NUMBER1
    #define MAX_ISR_TIMERS 3
#elif defined(ARDUINO_AVR_UNO)
    #define USE_ISR_TIMER_NUMBER1
    #define MAX_ISR_TIMERS 3
#else
     
#endif

namespace Chetch{
    
    class ISRTimer{

		public:
			enum class TimerMode{
              NOT_SET,
              COMPARE,
            };

            typedef void (*TimerListener)(uint8_t t);

            struct Callback {
                uint8_t id = 0;
                TimerListener onTimer = NULL;
            };


            static const int LOWEST_PRIORITY = -1;
            static const int HIGHEST_PRIORITY = -2;

            static const byte MAx_INSTANCES = MAX_ISR_TIMERS;
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
	
            
            Callback callbacks[MAX_CALLBACKS];
            Callback* singleCallback = NULL;
            uint16_t interruptCounts[MAX_CALLBACKS]; //how many times the interrupt is called before calling the callback
            uint16_t maxInterruptCount = 0; //calculated from comparison counts .. 
            byte callbackCount = 0;

            volatile unsigned long interruptDuration = 0;
            volatile unsigned long maxInterruptDuration = 0;

        private:
            int getCallbackIdx(uint8_t id);

		public:
            static ISRTimer *create(byte timerNumber, uint16_t prescaler, TimerMode mode = ISRTimer::TimerMode::COMPARE);
            static uint16_t gcd(uint16_t a, uint16_t b);

            //static inline bool freeToExecute(byte priority, uint16_t executionEnd);

            ISRTimer(byte timerNumber, uint16_t prescaler, TimerMode mode);

            bool addListener(uint8_t id, TimerListener listener, int priority, uint16_t comp = 0);
            bool removeListener(uint8_t id);
            uint16_t setCompareValue(uint8_t id, uint16_t comp);

            void onTimerInterrupt();

            bool isEnabled();
			void setCompareA(uint16_t cnt, uint16_t comp);
            void setCompareA(uint16_t comp);
            uint16_t getCompareA();
			void enable();
			void disable();
            
            uint32_t microsToTicks(uint32_t microseconds); //how many ticks of the timer
            uint32_t ticksToMicros(uint32_t ticks); //how many micros for a certin number of ticks
            uint32_t microsToInterrupts(uint8_t id, uint32_t microseconds); //how many interrupts are called in a given period of micros
            uint32_t interruptsToMicros(uint8_t id, uint32_t interrupts); //how many micros are a certain number of interrupts
    }; //end class
} //end namespae
#endif