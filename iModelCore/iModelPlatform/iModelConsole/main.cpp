/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/PlatformLib.h>
#include "iModelConsole.h"
#include <Bentley/Logging.h>

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#ifdef COMMENT_OUT_UNUSED_VARIABLE
static WCharCP s_configFileName = L"logging.config.xml";
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE_EC

// Function declaration
void fuzz(char *name);

std::string convertWCharToString(WCharCP wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsoleBeAssertHandler(wchar_t const* message, wchar_t const* file, unsigned line, BeAssertFunctions::AssertType atype)
    {
    WString errorMessage;
    errorMessage.Sprintf(L"ASSERTION FAILURE: %ls (%ls:%d)\n", message, file, line);

    NativeLogging::CategoryLogger("BeAssert").errorv(errorMessage.c_str());
    IModelConsole::WriteErrorLine(Utf8String(errorMessage).c_str());
    }

bool TryGetLogConfigPath(BeFileNameR logConfigPath, BeFileNameCR exeDir);

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InitLogging(BeFileNameCR exeDir)
    {
    }

// actual target function
void fuzz(char *name) {
    std::cout << "Inside Fuzz function" << std::endl;
    char *sample_bytes = NULL;
    uint32_t sample_size = 0; 
    
    // read the sample either from file
    FILE *fp = NULL;
    if (fopen_s(&fp, name, "rb") != 0 || !fp) {
        printf("Error opening %s\n", name);
        return;
    }
    fseek(fp, 0, SEEK_END);
    sample_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    sample_bytes = (char *)malloc(sample_size);
    fread(sample_bytes, 1, sample_size, fp);
    // Process the SQL input
    printf("Sample Data: %s\n", sample_bytes);

    IModelConsole::Singleton().ExecuteSampleQuery(sample_bytes);

    fclose(fp);

    if(sample_bytes)
        free(sample_bytes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain(int argc, WCharCP argv[])
    {
#ifdef _WIN32
#if defined (UNICODE_OUTPUT_FOR_TESTING)
    // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
    _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
    _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
#endif
#endif

    BeFileName exeDir = Desktop::FileSystem::GetExecutableDir();
    if (!exeDir.DoesPathExist())
        {
        BeAssert(false && "Exe path's directory should always exist.");
        return 1;
        }

    InitLogging(exeDir);

    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");

    //set customized assert handler as default handler causes exception which makes iModelConsole crash.
    BeAssertFunctions::SetBeAssertHandler(IModelConsoleBeAssertHandler);

    Dgn::PlatformLib::Initialize(IModelConsole::Singleton());

    std::string inputFileName = convertWCharToString(argv[1]);
    fuzz(const_cast<char*>(inputFileName.c_str()));

    return 0;
    // return IModelConsole::Singleton().ExecuteSampleQuery(argc, argv);
    }

#ifdef __unix__
extern "C" int main(int argc, char** argv) {
    BentleyApi::bvector<wchar_t*> argv_w_ptrs;
    for (int i = 0; i < argc; i++) {
        BentleyApi::WString argw(argv[i], BentleyApi::BentleyCharEncoding::Utf8);
        auto argp = new wchar_t[argw.size() + 1];
        wcscpy(argp, argw.data());
        argv_w_ptrs.push_back(argp);
    }

    return wmain(argc, (ARGV_TYPE)argv_w_ptrs.data());
}
#endif
