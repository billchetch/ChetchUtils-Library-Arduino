#include <ChetchUtils.h>

namespace Chetch{

char Utils::hexDigit(char c)
{  
  return "0123456789ABCDEF"[c & 0x0F];
}

char *Utils::urlencode(char *dst, char *src, const char *specialChars)
{  
   char *d = dst;
   char c;
   while (c = *src++)
   {  if (strchr(specialChars,c))
      {  *d++ = '%';
         *d++ = Utils::hexDigit(c >> 4);
         *d++ = Utils::hexDigit(c);
      }
      else *d++ = c;
   }
   *d = '\0';
   return dst;
}

char *Utils::urldecode(char *src){
  char *dst = src;
  while (*src) {
    if (*src == '+') {
      src++;
      *dst++ = ' ';
    }
    else if (*src == '%') {
      // handle percent escape

      *dst = '\0';
      src++;

      if (*src >= '0' && *src <= '9') {
        *dst = *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst = 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst = 10 + *src++ - 'a';
      }

      // this will cause %4 to be decoded to ascii @, but %4 is invalid
      // and we can't be expected to decode it properly anyway

      *dst <<= 4;

      if (*src >= '0' && *src <= '9') {
        *dst |= *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst |= 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst |= 10 + *src++ - 'a';
      }

      dst++;
    }
    else {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
  return src;
}

int Utils::parseNameValuePair(char *s2parse, const char *delimiter, char *results[], int maxResults, boolean decodeUrl) {
  int ct = 0;
  while (s2parse && *s2parse && ct < 2*maxResults) {
    results[ct] = strsep(&s2parse, delimiter);
    results[ct + 1] = strchrnul(results[ct], '=');
    if (*results[2*ct]) *results[ct + 1]++ = '\0';

    if (decodeUrl) {
      Utils::urldecode(results[ct]);
      Utils::urldecode(results[ct + 1]);
    }

    ct += 2;
  }
  return ct/2;
}

int Utils::parseQueryString(char *s2parse, char *results[], int maxResults, boolean decodeUrl){
  return Utils::parseNameValuePair(s2parse, "&", results, maxResults, decodeUrl);
}

char *Utils::getValue(const char *pname, char *results[], int resultsCount){
  for(int i = 0; i < 2*resultsCount; i+=2){
    if(strcasecmp(pname, results[i]) == 0)return results[i + 1];
  }

  return NULL;
}

void Utils::addValue(char *params[], char *pname, char *pvalue, int i){
  params[i] = pname;
  params[i + 1] = pvalue;
}

char *Utils::buildNameValueString(char *str, char *params[], int paramCount, const char *delimiter, boolean encodeUrl){
	char *param;
	char *value;
	char *buf1;
	char *buf2;
	if(encodeUrl){
		buf1 = new char[32];
		buf2 = new char[128];
	} 		
	for(int i = 0; i < 2*paramCount; i+=2){
		if(encodeUrl){
			Utils::urlencode(buf1, params[i]);
			Utils::urlencode(buf2, params[i + 1]);
			param = buf1;
			value = buf2;
			if (i == 0) {
				sprintf(str, "%s=%s", param, value);
			}
			else
			{
				sprintf(str, "%s%s%s=%s", str, delimiter, param, value);
			}
		}
		else 
		{
			param = params[i];
			value = params[i + 1];
			if (i == 0) {
				sprintf(str, "%s=%s", param, value);
			}
			else 
			{
				sprintf(str, "%s%s%s=%s", str, delimiter, param, value);
			}
		}
	}
	if(encodeUrl){
		delete[] buf1;
		delete[] buf2;
	}
	return str;
}

char *Utils::buildQueryString(char *str, char *params[], int paramCount, boolean encodeUrl){
  return Utils::buildNameValueString(str, params, paramCount, "&", encodeUrl);
}

char *Utils::getStringFromProgmem(char *buffer, byte idx, const char* const stringTable[]){
	//buffer[0] = 0;
	strcpy_P(buffer, (char *)pgm_read_word(&(stringTable[idx])));
	return buffer; 
}

unsigned int Utils::getUIntArrayFromProgmem(unsigned int *buffer, byte idx, const unsigned int* const intArrayTable[], const unsigned int intArrayLengths[]){
  unsigned int n = (unsigned int)pgm_read_word(&(intArrayLengths[idx]));
  for(unsigned int i = 0; i < n; i++){
    unsigned int value = (unsigned int)pgm_read_word(&(intArrayTable[idx][i]));
    buffer[i] = value;
  }
  return n;
}

} //end namespace