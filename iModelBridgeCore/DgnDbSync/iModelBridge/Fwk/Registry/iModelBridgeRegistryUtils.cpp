/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Registry/iModelBridgeRegistryUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux)
#include <unistd.h>
#endif
#include <iModelBridge/iModelBridgeRegistry.h>
#include <Bentley/BeGetProcAddress.h>

#define LOG (*LoggingManager::GetLogger(L"iModelBridgeRegistry"))

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2012
//---------------------------------------------------------------------------------------
static void justLogAssertionFailures(WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype)
    {
    WPrintfString str(L"ASSERT: (%ls) @ %ls:%u\n", message, file, line);
    LOG.error(str.c_str());
    //::OutputDebugStringW (str.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeRegistryUtils::InitCrt(bool quietAsserts)
    {
#ifdef NDEBUG
    quietAsserts = true; // we never allow disruptive asserts in a production program
#endif

    if (quietAsserts)
        BeAssertFunctions::SetBeAssertHandler(justLogAssertionFailures);

#ifdef _WIN32
    if (quietAsserts)
        _set_error_mode(_OUT_TO_STDERR);
    else
        _set_error_mode(_OUT_TO_MSGBOX);

    #if defined (UNICODE_OUTPUT_FOR_TESTING)
        // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
        _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
        _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
    #endif

    // FOR THE CONSOLE PUBLISHER ONLY! "Gui" publishers won't have any console output and won't need this.
    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");
#else
    // unix-specific CRT init
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void* iModelBridgeRegistryUtils::GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName)
    {
    BeFileName removeCurrentDir(L"");
    BeGetProcAddress::SetLibrarySearchPath(removeCurrentDir);
    BeFileName pathname(BeFileName::FileNameParts::DevAndDir, bridgeDllName);

    BeGetProcAddress::SetLibrarySearchPath(pathname);
    auto hinst = BeGetProcAddress::LoadLibrary(bridgeDllName);
    if (!hinst)
        {
        LOG.fatalv(L"%ls: not found or could not be loaded", bridgeDllName.c_str());
        return nullptr;
        }

    return BeGetProcAddress::GetProcAddress (hinst, funcName);
    }

