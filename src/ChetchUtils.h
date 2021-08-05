#ifndef CHETCH_UTILS_H
#define CHETCH_UTILS_H

#include <Arduino.h>

namespace Chetch{

class Utils{
  private:
    static char hexDigit(char c);
    
  public:
    static char *urlencode(char* dst, char* src, const char *specialChars = "$&+,/:;=?@ <>#%{}|~[]`");
    static char *urldecode(char* src);
    static int parseNameValuePair(char *s2parse, const char *delimiter, char *results[], int maxResults, boolean decodeUrl);
    static int parseQueryString(char *s2parse, char *results[], int maxResults, boolean decodeUrl = true);
    static char *getValue(const char *pname, char *results[], int resultsCount);
    static void addValue(char *params[], char *pname, char *pvalue, int i);
    static char *buildNameValueString(char *str, char *params[], int paramCount, const char *delimiter, boolean encodeUrl);
    static char *buildQueryString(char *str, char *params[], int paramCount, boolean encodeUrl = true);
	
    static char *getStringFromProgmem(char *buffer, byte idx, const char* const stringTable[]);
    static unsigned int getUIntArrayFromProgmem(unsigned int *buffer, byte idx, const unsigned int* const intArrayTable[], const unsigned int intArrayLengths[]);

    template <typename T> static T bytesTo(byte *bytes, int numberOfBytes, bool littleEndian){
        //TODO: cater for not littleEndian
        T retVal = 0;
        for(int i = 0; i < numberOfBytes; i++){
            ((byte *)&retVal)[i] = bytes[i];
        }
        return retVal;
    };

    template <typename T> static void toBytes(T t, byte *bytes, bool littleEndian){
        //TODO: cater for not littleEndian
        for(int i = 0; i < sizeof(t); i++){
          bytes[i] = ((byte*)&t)[i]; 
        }
    }
};

} //end namespace

#endif