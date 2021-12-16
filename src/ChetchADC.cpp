#include "ChetchADC.h"

namespace Chetch{
    void CADC::init(AnalogReference areference){
        switch(areference){
            case AnalogReference::AREF_EXTERNAL:
                ADMUX &= ~((1 << REFS0) | (1 << REFS1)); //set external ref by turning off REFSI1 and REFS2
                break;

            case AnalogReference::AREF_INTERNAL:
                ADMUX &= ~(1 << REFS1); //turn off REFS1 bit
                ADMUX |= (1 << REFS0);  //turn on REFS0 bit
                //TODO: set register
                break;

            case AnalogReference::AREF_INTERNAL1V1:
                //TODO: set register
                ADMUX |= (1 << REFS1); //turn on REFS1 bit
                ADMUX &= ~(1 << REFS0);  //turn off REFS0 bit
                break;

            case AnalogReference::AREF_INTERNAL2V56:
                ADMUX |= (1 << REFS1); //turn on REFS1 bit
                ADMUX |= (1 << REFS0);  //turn on REFS0 bit
                break;

        }
    }

    void CADC::init(bool triggerInterrupt){
        if(triggerInterrupt){
            //TODO: set ADCRSA register for interrupts
        } else {
            //turn off trigger
            ADCSRA |= (1 << ADEN) | (0 << ADSC) | (0 << ADATE); //auto-trigger OFF
        }
        ADCSRB = 0x00;
    }

    void CADC::init(AnalogReference areference, bool triggerInterrupt){
        init(areference);
        init(triggerInterrupt);
    }

    CADC::AnalogReference CADC::aref(){
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
        }
    }

    bool CADC::isReading(){
        return (ADCSRA & (1 << ADSC)) > 0;
    }

    void CADC::startRead(int analogPin){
        if(analogPin - A0 >= 0)analogPin = analogPin - A0;

        //set the pin
        ADMUX |= analogPin & 0x07; //value of 7 ensures only first 3 bits of ADMUX register are set

        //start the read
        ADCSRA |= (1 << ADSC);
    }

    uint16_t CADC::readResult(){
        return (ADCL | (ADCH << 8)); 
    }
} //end namespace
