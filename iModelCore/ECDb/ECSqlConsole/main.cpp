/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/main.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Logging/BentleyLogging.h>
#include <ECDb/ECDbApi.h>
#include "ECSqlConsole.h"
#include "Console.h"
//#include <BeXml/BeXml.h>

static WCharCP s_configFileName = L"logging.config.xml";

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.eberle     07/2013
//---------------------------------------------------------------------------------------
void ECSqlConsoleBeAssertHandler (wchar_t const* message, wchar_t const* file, unsigned line, BeAssertFunctions::AssertType atype)
    {
     
    WString errorMessage;
    errorMessage.Sprintf (L"ASSERTION FAILURE: %ls (%ls:%d)\n", message, file, line);
    Utf8String errorMessageUtf8 (errorMessage.c_str ());

    Bentley::NativeLogging::LoggingManager::GetLogger (L"BeAssert")->errorv (errorMessage.c_str ());
    Console::WriteErrorLine (errorMessageUtf8.c_str ());
    }

bool TryGetLogConfigPath (BeFileNameR logConfigPath, WCharCP exePath);
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2014
//---------------------------------------------------------------------------------------
void InitLogging (WCharCP exeName)
    {
    BeFileName configFilePath;
    if (TryGetLogConfigPath (configFilePath, exeName))
        {
        NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_CONFIG_FILE, configFilePath);
        NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2014
//---------------------------------------------------------------------------------------
bool TryGetLogConfigPath (BeFileNameR logConfigPath, WCharCP exePathStr)
    {
    Utf8String configFilePathStr (getenv ("ECSQLCONSOLE_LOGGING_CONFIG"));
    logConfigPath = BeFileName (configFilePathStr);
    if (!configFilePathStr.empty () && logConfigPath.DoesPathExist ())
        return true;

    BeFileName exePath (exePathStr);
    logConfigPath = exePath.GetDirectoryName ();
    logConfigPath.AppendToPath (L"logging.config.xml");
    logConfigPath.BeGetFullPathName ();

    return logConfigPath.DoesPathExist ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2012
//---------------------------------------------------------------------------------------
int wmain (int argc, WCharP argv[])
    {
#if defined (UNICODE_OUTPUT_FOR_TESTING)
    // turning this on makes it so we can show unicode characters, but screws up piped output for programs like python.
    // Since console output is not a production concept anyway, is merely for testing.
    _setmode(_fileno(stdout), _O_U16TEXT);  // so we can output any and all unicode to the console
    _setmode(_fileno(stderr), _O_U16TEXT);  // so we can output any and all unicode to the console
#endif

    InitLogging (argv[0]);

    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside 
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is 
    //based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale. 
    // However, the success of this code is dependent on two things: 
    //      1) The narrow encoding must support the wide character being converted. 
    //      2) The font/gui must support the rendering of that character. 
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console." 
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale(LC_CTYPE, "");

    Utf8String tempDirUtf8 (getenv ("TEMP"));
    if (tempDirUtf8.empty ())
        { 
        Console::WriteErrorLine ("Environment variable TEMP not set. Please set it to a temporary directory and restart the application.");
        return 1;
        }

    BeFileName tempDir (tempDirUtf8);
    if (argv[0])
        {
        auto str = BeFileName::GetDirectoryName(argv[0]);
        if (!str.empty ())
            {
            BeFileName dbDir (str);
            ECDb::Initialize (tempDir, &dbDir);
            }
        }
    else
        ECDb::Initialize (tempDir, nullptr); 

    //set customized assert handler as default handler causes exception which makes console crash.
    BeAssertFunctions::SetBeAssertHandler (ECSqlConsoleBeAssertHandler);
    return ECSqlConsole::Run( argc, argv);
    }