/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/atp/log4cxxTest.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <cppunittest/TestHarness.h>
#include <windows.h>
#include <bentley/bentley.h>
#include <log4cxx.h>

USING_NAMESPACE_CPPUNITTEST;
USING_NAMESPACE_BENTLEY_LOGGING;

TEST (Log4cxxProvider, construction)
        {
        std::vector<std::wstring> fileNames;

        Provider::Log4cxxProvider *pLogProvider = new Provider::Log4cxxProvider ();
        CHECK ( NULL != pLogProvider );

    pLogProvider->GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

    delete pLogProvider;

        pLogProvider = new Provider::Log4cxxProvider ( NULL );
        CHECK ( NULL != pLogProvider );

    pLogProvider->GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

    delete pLogProvider;

        pLogProvider = new Provider::Log4cxxProvider ( L"asfdasdf" );
        CHECK ( NULL != pLogProvider );

        pLogProvider->GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

    delete pLogProvider;

        Provider::Log4cxxProvider logProvider1;
        logProvider1.GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

        Provider::Log4cxxProvider logProvider2 ( NULL );
        logProvider2.GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

        Provider::Log4cxxProvider logProvider3 ( L"zxvxzvxzcv" );
        logProvider3.GetLogfileNames ( fileNames );
        CHECK ( true == fileNames.empty() );

        }

TEST (Log4cxxProvider, configurationSUCCESS)
        {
        Provider::Log4cxxProvider l4cxx;
        LPCWSTR cfgFile = L"logtest.log.xml";

        std::vector<std::wstring> fileNames;
        CHECK ( SUCCESS == l4cxx.GetLogfileNames(fileNames) );
        CHECK ( true == fileNames.empty() );

        CHECK ( SUCCESS == l4cxx.BasicConfiguration() );
        CHECK ( SUCCESS == l4cxx.GetLogfileNames(fileNames) );
        CHECK ( true == fileNames.empty() );

        CHECK ( SUCCESS != l4cxx.LoadConfiguration ( cfgFile ) );
        CHECK ( SUCCESS == l4cxx.GetLogfileNames(fileNames) );
        CHECK ( true == fileNames.empty() );

        CHECK ( SUCCESS != l4cxx.LoadAndWatchConfiguration( cfgFile, 6000 ) );
        }

TEST (Log4cxxProvider, configurationFAILURE)
    {
        Provider::Log4cxxProvider l4cxx;
        std::vector<std::wstring> fileNames;

        START_EXCEPT( L"Invalid Args: NULL Configuration file" );
        l4cxx.LoadConfiguration( NULL );
        END_WITH_EXCEPT( L"End Inalid Args" );

        // NEEDSWORK
        //START_EXCEPT( L"Invalid Args: logtest2.log.xml - no dtd file" );
        CHECK ( SUCCESS != l4cxx.LoadConfiguration( L"logtest2.log.xml" ) );
        //END_WITH_EXCEPT( L"End Inalid Args" );

        //START_EXCEPT( L"Invalid Args: logtest3.log.xml - invalid file name" );
        CHECK ( SUCCESS != l4cxx.LoadConfiguration( L"logtest3.log.xml" ) );
        //END_WITH_EXCEPT( L"End Inalid Args" );

        CHECK ( SUCCESS == l4cxx.GetLogfileNames(fileNames) );
        CHECK ( true == fileNames.empty() );
    }

TEST (Log4cxxProvider, threadsNDCFAILURE)
        {
        Provider::Log4cxxProvider l4cxx;
        LPCWSTR context = L"log4cxxTestContext";

        // NEEDSWORK: There is no error returned for one too many pops
        START_EXCEPT( L"One too many pops" );
        l4cxx.PushThreadContext(context);
        l4cxx.PopThreadContext();
        l4cxx.PopThreadContext();
        l4cxx.ClearThreadContext();
        END_NO_EXCEPT( L"One too many pops" );
        }

TEST (Log4cxxProvider, threadsMDCFAILURE)
        {
        Provider::Log4cxxProvider l4cxx;
        LPCWSTR context = L"log4cxxTestContext";
        LPCWSTR key = L"log4cxxTestKey";

        // NEEDSWORK: There is no error returned for one too many pops
        START_EXCEPT( L"One too many pops" );
        l4cxx.AddContext(key, context);
        l4cxx.RemoveContext(key);
        l4cxx.RemoveContext(key);
        l4cxx.ClearContext();
        END_NO_EXCEPT( L"One too many pops" );
        }

TEST (Log4cxxProvider, Logger)
        {
        Provider::Log4cxxProvider f;
        LPCWSTR nameSpace = L"Log4cxxLoggerTest";

        Provider::ILogProviderContext *pContext = new Provider::ILogProviderContext();

    CHECK ( SUCCESS == f.CreateLogger(nameSpace, &pContext));

        CHECK ( SUCCESS == f.SetSeverity( nameSpace, LOG_FATAL ) );
        CHECK ( true == f.IsSeverityEnabled(pContext, LOG_FATAL) );

        CHECK ( SUCCESS == f.SetSeverity( nameSpace, LOG_DEBUG ) );
        CHECK ( true == f.IsSeverityEnabled( pContext, LOG_DEBUG ) );
        CHECK ( false == f.IsSeverityEnabled( pContext, (SEVERITY)-1000 ) );

        // NEEDSWORK: Invalid severity makes it through
        START_EXCEPT("Invalid Args: SEVERITY");
        f.LogMessage(pContext, (SEVERITY)-1000, L"This is from the Log4cxxLogger Test");
        END_NO_EXCEPT("Invalid Args");

        CHECK ( SUCCESS == f.DestroyLogger (pContext) );
        }