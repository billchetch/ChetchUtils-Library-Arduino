#include "ChetchADC.h"

namespace Chetch{
    static void CADC::init(AnalogReference ref){
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

            case AnalogReference::AREF_INTERNAL1V1:
                //TODO: set register
                break;

            case AnalogReference::AREF_INTERNAL2V56:
                //TODO: set register
                break;

        }
    }

    static void CADC::init(bool triggerInterrupt){
        if(triggerInterrupt){
            //TODO: set ADCRSA register for interrupts
        } else {
            //turn off trigger
            ADCSRA |= (1 << ADEN) | (0 << ADSC) | (0 << ADATE); //auto-trigger OFF
        }
        ADCSRB = 0x00;
    }

    static void CADC::init(AnalogReference ref, bool triggerInterrupt){
        init(ref);
        init(triggerInterrupt);
    }

    static CADC::AnalogReference CADC::aref(){
        //read the ADMUX register REFS0 and REFS1 bits
        switch(ADMUX >> REFS0){
            case 0: //External
                //Serial.println("External"); 
                return AnalogReference::AREF_EXTERNAL;

            case 1: //Internal
                //Serial.println("Internal"); 
                return AnalogReference::AREF_INTERNAL;

            case 2: //Internal1V1
                //Serial.println("Internal1v1"); 
                return AnalogReference::AREF_INTERNAL1V1;

            case 3: //Internal2v56
                //Serial.println("Internal2v56");
                return AnalogReference::AREF_INTERNAL2V56;

            default:
                //Serial.println("Default");
                return AnalogReference::AREF_DEFAULT;
        }
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
