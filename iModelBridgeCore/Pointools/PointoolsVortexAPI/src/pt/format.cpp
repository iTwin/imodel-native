/**
 @file format.cpp

 @author Morgan McGuire, graphics3d.com

 @created 2000-09-09
 @edited  2004-01-03
*/

#include <pt/format.h>
#include <pt/ptunicode.h>

#ifdef WIN32
    #include <windows.h>
    #define vsnprintf _vsnprintf
    #define NEWLINE "\r\n"
#else
    #include <stdarg.h>
    #define NEWLINE "\n"
#endif

#ifdef _MSC_VER
    // disable: "C++ exception handler used"
    #pragma warning (disable : 4530)
#endif // _MSC_VER

// If your platform does not have vsnprintf, you can find a
// implementation at http://www.ijs.si/software/snprintf/

namespace pt {

std::string __cdecl format(const char* fmt,...) {
    va_list argList;
    va_start(argList,fmt);
    std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}

#if defined(WIN32) &&  (_MSC_VER < 1300)
// MSVC 6 uses the pre-C99 vsnprintf, which has different behavior
std::string vformat(const char *fmt, va_list argPtr) {
    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
	char stackBuffer[bufSize];

    int attemptedSize = bufSize - 1;

    int numChars = vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);

    if (numChars >= 0) {
        // Got it on the first try.
        return std::string(stackBuffer);
    }

    // Now use the heap.
    char* heapBuffer = NULL;

    while ((numChars == -1) && (attemptedSize < maxSize)) {
        // Try a bigger size
        attemptedSize *= 2;
        heapBuffer = (char*)realloc(heapBuffer, attemptedSize + 1);
        numChars = vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
    }

    std::string result(heapBuffer);

    free(heapBuffer);

    return result;

}

#else

// glibc 2.1 and MSVC 7 have been updated to the C99 standard
std::string vformat(const char* fmt, char* argPtr) {
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

#endif

} // namespace

#ifdef WIN32
  #undef _vsnprintf
#endif

#undef NEWLINE
