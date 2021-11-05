#include "ChetchADC.h"

namespace Chetch{
    static void CADC::init(AnalogReference ref, bool triggerInterrupt){
        switch(ref){
            case AnalogReference::AREF_DEFAULT:
                //leave everything alone
                break;

            case AnalogReference::AREF_EXTERNAL:
                ADMUX &= ~((1 << REFS0) | (1 << REFS1)); //set external ref
                break;

            case AnalogReference::AREF_INTERNAL:
                //TODO: set register
                break;
        }

        
        if(triggerInterrupt){
            //TODO: set ADCRSA register for interrupts
        } else {
            //turn off trigger
            ADCSRA |= (1 << ADEN) | (0 << ADSC) | (0 << ADATE); //auto-trigger OFF
        }
        ADCSRB = 0x00;
    }

    static bool CADC::isReading(){
        return (ADCSRA & (1 << ADSC)) > 0;
    }

    static void CADC::startRead(int analogPin){
        if(analogPin - A0 >= 0)analogPin = analogPin - A0;

        //set the pin
        ADMUX |= analogPin & 0x07; //value of 7 ensures only first 3 bits of ADMUX register are set

        //start the read
        ADCSRA |= (1 << ADSC);
    }

    static uint16_t CADC::readResult(){
        return (ADCL | (ADCH << 8)); 
    }
} //end namespace
