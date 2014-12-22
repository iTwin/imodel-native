/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/native/atp/atp.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "log4cxx.h"
#include "assert.h"

USING_NAMESPACE_BENTLEY_LOGGING;

void myPurecallHandler(void)
    {
    DebugBreak();
    assert ( !"We had a virtual function call" );
    printf("In _purecall_handler.");
    exit(0);
    }

int wmain(int argc, wchar_t* argv[])
    {
    _set_error_mode(_OUT_TO_MSGBOX);
    _set_purecall_handler(myPurecallHandler);
    assert ( !"We are started!!!" );

    ILogger* pLogger = LoggerRegistry::getLogger ( L"TESTNS" );

    Provider::Log4cxxProvider* pLog = new Provider::Log4cxxProvider;

    pLog->LoadConfiguration(L"log.xml");
//    pLog->BasicConfiguration();

    LoggerConfig::registerLogInterface ( pLog );

    ENABLE_SCOPE_LOGGING(pLogger);

    pLogger->message ( LOG_FATAL, L"Fatal Log message 1" );
    pLogger->messagev ( LOG_FATAL, L"Fatal Log message %d %s", 234, L"inserted string" );

        {
        ENABLE_SCOPE_LOGGING(pLogger);
        }

    LoggerConfig::setMaxMessageSize ( 10 );

    pLogger->message ( LOG_ERROR, L"Error Log message 1" );
    pLogger->messagev ( LOG_ERROR, L"Error Log message %d %s", 234, L"inserted string" );

    pLog->PushThreadContext ( L"Some context" );

    pLogger->message ( LOG_FATAL, L"Test Log message 3" );
    pLogger->message ( LOG_FATAL, L"Test Log message 4" );

    pLog->ClearThreadContext();

    pLogger = LoggerRegistry::getLogger ( L"TESTNS2" );

    pLogger->message ( LOG_ERROR, L"Test Log message 4" );
    pLogger->message ( LOG_WARNING, L"Test Log message 4" );
    pLogger->message ( LOG_INFO, L"Test Log message 4" );
    pLogger->message ( LOG_DEBUG, L"Test Log message 4" );
    pLogger->message ( LOG_TRACE, L"Test Log message 4" );

    pLogger->message ( LOG_TRACE, L"Test Log message 4" );
    pLogger->message ( LOG_DEBUG, L"Test Log message 4" );
    pLogger->message ( LOG_INFO, L"Test Log message 4" );
    pLogger->message ( LOG_WARNING, L"Test Log message 4" );
    pLogger->message ( LOG_ERROR, L"Test Log message 4" );
    pLogger->message ( LOG_FATAL, L"Test Log message 4" );

    LoggerConfig::unregisterLogInterface ( );

    delete pLog;

    pLogger->message ( LOG_TRACE, L"Test Log message 4" );
    pLogger->message ( LOG_DEBUG, L"Test Log message 4" );
    pLogger->message ( LOG_INFO, L"Test Log message 4" );
    pLogger->message ( LOG_WARNING, L"Test Log message 4" );
    pLogger->message ( LOG_ERROR, L"Test Log message 4" );
    pLogger->message ( LOG_FATAL, L"Test Log message 4" );

    pLogger = LoggerRegistry::getLogger ( L"TESTNS3" );

    pLogger->message ( LOG_TRACE, L"Test Log message 4" );
    pLogger->message ( LOG_DEBUG, L"Test Log message 4" );
    pLogger->message ( LOG_INFO, L"Test Log message 4" );
    pLogger->message ( LOG_WARNING, L"Test Log message 4" );
    pLogger->message ( LOG_ERROR, L"Test Log message 4" );
    pLogger->message ( LOG_FATAL, L"Test Log message 4" );

//    LoggerFactory::destroyLogger ( pLogger );


    return 0;
    }

