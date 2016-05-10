/**
 @file format.cpp

 @author Morgan McGuire, graphics3d.com

 @created 2000-09-09
 @edited  2004-01-03
*/

#include "PointoolsVortexAPIInternal.h"
#include <pt/format.h>
#include <pt/ptunicode.h>

// If your platform does not have vsnprintf, you can find a
// implementation at http://www.ijs.si/software/snprintf/

namespace pt {

std::string CDECL_ATTRIBUTE format(const char* fmt,...) {
    va_list argList;
    va_start(argList,fmt);
    std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}


// glibc 2.1 and MSVC 7 have been updated to the C99 standard
std::string vformat(const char* fmt, va_list argPtr) {
    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.  The number 161 is chosen
    // to support two lines of text on an 80 character
    // console (plus the null terminator).
    const int bufSize = 161;
    char stackBuffer[bufSize];

    int numChars = vsnprintf(stackBuffer, bufSize, fmt, argPtr);

    if (numChars > bufSize) {
      // We didn't allocate a big enough string.
      char* heapBuffer = (char*) malloc(numChars * sizeof(char));

      assert(heapBuffer);
      int numChars2 = vsnprintf(heapBuffer, numChars, fmt, argPtr);
      assert(numChars2 == numChars);

      std::string result(heapBuffer);
      
      free(heapBuffer);

      return result;

    } else {

      return std::string(stackBuffer);

    }
}


} // namespace


