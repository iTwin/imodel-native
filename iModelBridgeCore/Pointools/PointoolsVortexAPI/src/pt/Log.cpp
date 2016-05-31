/**
  @file Log.cpp

  @maintainer Morgan McGuire, morgan@graphics3d.com
  @cite       Backtrace by Aaron Orenstein
  @created 2001-08-04
  @edited  2003-08-04

  @feb 07 - unicode support added by Faraz Ravi
 */

#include "PointoolsVortexAPIInternal.h"
#include <pt/Log.h>
#include <pt/format.h>
#include <pt/Array.h>
#include <ptfs/filepath.h>
#include <pt/ptmath.h> 
#include <time.h>

namespace pt {

Log* Log::commonLog = NULL;

Log::Log(const std::wstring& filename, int stripFromStackBottom)
    : stripFromStackBottom(stripFromStackBottom)
    {

    this->filename = filename;

    AString filenameA(filename.c_str());
    logFile = fopen(filenameA.c_str(), "w");

    if (logFile == NULL)
        {
        std::string logName;

        // Write time is greater than 1ms.  This may be a network drive.... try another file.
#ifdef _WIN32
        if (ptds::FilePath::checkExists(L"c:/tmp"))
            {
            logName = std::string("c:/tmp/") + logName;
            }
        else if (ptds::FilePath::checkExists(L"c:/temp"))
            {
            logName = std::string("c:/temp/") + logName;
            }
        else
            {
            logName = std::string("c:/") + logName;
            }
#else
        logName = std::string("/tmp/") + logName;
#endif

        logFile = fopen(logName.c_str(), "w");
        }

    // Turn off buffering.
    setvbuf(logFile, NULL, _IONBF, 0);

    fprintf(logFile, "Application Log\n");
    time_t t;
    time(&t);
    fprintf(logFile, "Start: %s\n", ctime(&t));
    fflush(logFile);

    if (commonLog == NULL)
        {
        commonLog = this;
        }
    }


Log::~Log() {
    section(L"Shutdown");
    println(L"Closing log file");
    
    // Make sure we don't leave a dangling pointer
    if (Log::commonLog == this) {
        Log::commonLog = NULL;
    }

    fclose(logFile);
}


FILE* Log::getFile() const {
    return logFile;
}

Log* Log::common() {
    if (commonLog == NULL) {
        commonLog = new Log();
    }
    return commonLog;
}


std::wstring Log::getCommonLogFilename() {
    return common()->filename;
}


void Log::section(const std::wstring& s) {
    fwprintf(logFile, L"_____________________________________________________\n");
    fwprintf(logFile, L"\n    ###    %s    ###\n\n", s.c_str());
}


void CDECL_ATTRIBUTE Log::printf(const char* fmt, ...) {
    printHeader();
	va_list arg_list;
	va_start(arg_list, fmt);

    printHeader();
    fprintf(logFile, "%s", vformat(fmt, arg_list).c_str());
    va_end(arg_list);
}


void Log::print(const std::wstring& s) {
    printHeader();
    fwprintf(logFile, L"%s", s.c_str());
}


void Log::println(const std::wstring& s) {
    printHeader();
    fwprintf(logFile, L"%s\n", s.c_str());
}


/* &&ep delete (iOS) ?

// clang reports that this function is unused, but the call to it is commented out below, so we just disable the warning for now.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

*/

/**
 Attempts to produce the stack frame list as a string.

  @param stripFromTop Number of stack frames on the top of the stack to hide.
 */

/* 
static std::string getBacktrace(
    int maxFrames,
    int stripFromTop = 0,
    int stripFromBottom = 0) {
#ifndef _WIN64
    #ifdef _MSC_VER

        Array<std::string> trace;

	    HANDLE process = GetCurrentProcess();
	    BOOL success = SymInitialize(process, NULL, true);

	    int _ebp;
	    __asm { mov _ebp,ebp } 

	    int frame = 0;
	    while (frame < maxFrames) {
		    int pc = ((int*)_ebp)[1];
		    _ebp = ((int*)_ebp)[0];
            
            if (pc == 0) {
                break;
            }

		    char csymbol[sizeof(IMAGEHLP_SYMBOL) + 256];
		    memset(csymbol, 0, sizeof(csymbol));

		    IMAGEHLP_SYMBOL* symbol = (IMAGEHLP_SYMBOL*)csymbol;
		    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
		    symbol->MaxNameLength = 256;

		    success = SymGetSymFromAddr(process, pc, 0, symbol);
            if (! success) {
                break;
            }

            trace.append(symbol->Name);

		    ++frame;
	    }


        std::string result;
        for (int i = trace.size() - stripFromBottom - 1; i >= stripFromTop; --i) {
            result += trace[i];
            if (i != stripFromTop) {
                result += " > ";
            }
        }

        return result;

    #else

        // On non-MSVC, just return the empty string
        return "";

    #endif
#else
		return "";
#endif
}

*/

/* &&ep delete on iOS ?

#ifdef __clang__
#pragma clang diagnostic pop
#endif
*/

void Log::printHeader() {
    time_t t;
    if (time(&t) != ((time_t)-1)) {
        /*
        char buf[32];
        strftime(buf, 32, "[%H:%M:%S]", localtime(&t));
    
         Removed because this doesn't work on SDL threads.

        #ifdef _DEBUG
            std::string bt = getBacktrace(15, 2, stripFromStackBottom);
            fprintf(logFile, "\n %s %s\n\n", buf, bt.c_str());
        #endif

        fprintf(logFile, "\n %s \n", buf);
        */

    } else {
        println(L"[Error getting time]");
    }
}

}
