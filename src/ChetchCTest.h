#ifndef CHETCH_CTEST_H
#define CHETCH_CTEST_H

#include <Arduino.h>

#ifndef CTEST_VAL
#define CTEST_VAL 10
#endif

//#pragma message("Compiling in ChetchCTest.cpp")
//hmm

namespace Chetch{
    class CTest{

        public:
            int k = CTEST_VAL;

            int getK();
    };
}
#endif
