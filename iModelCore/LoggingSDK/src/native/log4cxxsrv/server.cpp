/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/native/log4cxxsrv/server.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*
 * Copyright 2003,2004 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/config.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/serversocket.h>
#include <log4cxx/helpers/socket.h>
#include <log4cxx/net/socketnode.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/thread.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/stringhelper.h>

#include <bentley.h>
#include "servhlpr.h"

using namespace log4cxx;
#ifdef HAVE_XML
using namespace log4cxx::xml;
#endif
using namespace log4cxx::net;
using namespace log4cxx::helpers;

static const std::wstring   LOGSERVICE_NAMESPACE = L"bsi.logserver";
static const std::wstring   LOGSERVICE_NAME = L"Bentley Log Server";

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class Config
{
public:
    enum Action
        {
        RUN,
        INSTALL,
        UNINSTALL
        };

    int             port;
    bool            console;
    bool            autoStart;
    Action          action;
    std::wstring    configFile;

    Config ( void ) : port ( 5700 ), autoStart ( true ), console ( false ), action ( RUN ) {};

    virtual ~Config ( void ) {};

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Config   g_config;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int WINAPI LoadConfig
(
Config&     config
)
    {
    try
        {
        if ( config.configFile.empty() )
            {
            BasicConfigurator::configure();
            }
#ifdef HAVE_XML
        // tests if configFile ends with ".xml"
        else if (StringHelper::endsWith(config.configFile, _T(".xml")))
                {
            DOMConfigurator::configure(config.configFile);
                }
#endif
        else
                {
            PropertyConfigurator::configure(config.configFile.c_str());
                }
        }
    catch ( Exception& )
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DWORD WINAPI ServerThread
(
LPVOID lpParameter
)
    {
    int     status = SUCCESS;
    HANDLE  service = (HANDLE)lpParameter;

    ServiceHelper*  pHelper = ServiceHelper::Instance();

    pHelper->UpdateServiceStatus ( service, SERVICE_STATUS_RUNNING, SUCCESS, 30000 );

    try
        {
        LoggerPtr logger = Logger::getLogger(LOGSERVICE_NAMESPACE);

        LOG4CXX_INFO(logger, _T("Listening on port ") << g_config.port);

        ServerSocket serverSocket(g_config.port);

        while(true)
            {
            LOG4CXX_INFO(logger, _T("Waiting to accept a new client."));
            SocketPtr socket = serverSocket.accept();

            LOG4CXX_INFO(logger, _T("Connected to client at ")
                << socket->getInetAddress().toString());
            LOG4CXX_INFO(logger, _T("Starting new socket node."));

            Thread * pThread = new Thread(new SocketNode(socket,
                LogManager::getLoggerRepository()));
            pThread->start();
            }
        }
    catch(SocketException& e)
        {
                tcout << _T("SocketException: ") << e.getMessage() << std::endl;
        status = ERROR;
//              LoggerPtr logger = Logger::getLogger(LOGSERVICE_NAMESPACE);

//                LOG4CXX_ERROR ( logger, _T("SocketException: ") << e.getMessage() );
        }
    catch(Exception& e)
        {
        tcout << _T("Exception: ") << e.getMessage() << std::endl;
        status = ERROR;
//              LoggerPtr logger = Logger::getLogger(LOGSERVICE_NAMESPACE);

//                LOG4CXX_ERROR ( logger, _T("Exception: ") << e.getMessage() );
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WINAPI serviceStart
(
HANDLE  service,
DWORD   argc,
LPTSTR* argv
)
    {
    ServiceHelper*  pHelper = ServiceHelper::Instance();

    pHelper->UpdateServiceStatus ( service, SERVICE_STATUS_START_PEND, SUCCESS, 30000 );

    DWORD   threadId = 0;

    HANDLE hThread = ::CreateThread ( NULL, 0, ServerThread, service, 0, &threadId );

    ::CloseHandle ( hThread );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int WINAPI serviceStop
(
HANDLE              service,
ServiceEventType    type
)
    {
    ServiceHelper*  pHelper = ServiceHelper::Instance();

    pHelper->UpdateServiceStatus ( service, SERVICE_STATUS_STOP_PEND, SUCCESS, 30000 );


    pHelper->UpdateServiceStatus ( service, SERVICE_STATUS_STOPPED, SUCCESS, 30000 );

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WINAPI PrintUsage ( int status )
    {
    printf ( "\nUsage:\n" );
    printf ( "\t-p portNumber : Port number to listen on\n" );
    printf ( "\t-f configFile : Name of config file to load, should be xml \n" );
    printf ( "\t-c            : Run in console mode\n" );
    printf ( "\t-i            : Install as a service\n" );
    printf ( "\t-m            : Install service in manual start mode\n" );
    printf ( "\t-u            : Uninstall the service\n\n" );
    exit(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int WINAPI ParseCommandLine
(
int         argc,
wchar_t*    argv[],
Config&     config
)
    {
    WCHAR       moduleName[FILENAME_MAX] = {0};

    // Set the defaults
    config.action = Config::RUN;
    config.port = 5700;
    config.autoStart = true;
    config.console = false;

    ::GetModuleFileName ( NULL, moduleName, sizeof(moduleName)/sizeof(moduleName[0]) );

    WCHAR* pChar = wcsrchr ( moduleName, '.' );

    if ( NULL != pChar )
        {
        *pChar = NULL;
        }

    wcscat ( moduleName, _T(".xml"));

    config.configFile = moduleName;

    // Look at the args and see what to do
    for ( int i=1 ; i<argc ; i++ )
        {
        // We're looking for an option flag
        if ( '-' != argv[i][0] )
            {
            PrintUsage(ERROR);
            }

        if ( 'p' == argv[i][1] )
            {
            // Set the port number
            if ( i+1>argc )
                {
                PrintUsage(ERROR);
                }

            config.port = ttol(argv[i+1]);

            if ( 0 == config.port )
                {
                PrintUsage(ERROR);
                }

            i++;
            }
        else if ( 'f' == argv[i][1] )
            {
            // Set the config file
            if ( i+1>argc )
                {
                PrintUsage(ERROR);
                }

            config.configFile = argv[i+1];

            i++;
            }
        else if ( 'c' == argv[i][1] )
            {
            // Run in console mode
            config.console = true;
            }
        else if ( 'm' == argv[i][1] )
            {
            // Install the service in manual start mode
            config.autoStart = false;
            }
        else if ( 'i' == argv[i][1] )
            {
            // Install the service
            config.action = Config::INSTALL;
            }
        else if ( 'u' == argv[i][1] )
            {
            // Uninstall the service
            config.action = Config::UNINSTALL;
            }
        else if ( 'h' == argv[i][1] )
            {
            // Dispay the usage help
            PrintUsage(SUCCESS);
            }
        else
            {
            // We don't have a valid option, get out
            PrintUsage(ERROR);
            }
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain
(
int         argc,
wchar_t*    argv[]
)
    {
        int             status = SUCCESS;
        HANDLE          service = 0;
        ServiceHelper*  pHelper = ServiceHelper::Instance();

        status = pHelper->RegisterService ( LOGSERVICE_NAME, serviceStart, &service );

        status = pHelper->RegisterHandler ( service, SERVICE_EVENT_STOP, serviceStop );

    status = ParseCommandLine ( argc, argv, g_config );

    if ( SUCCESS != status )
        {
        return status;
        }

    if ( Config::INSTALL == g_config.action )
        {
        status = pHelper->InstallServices( g_config.autoStart );
        }
    else if ( Config::UNINSTALL == g_config.action )
        {
        status = pHelper->UninstallServices();
        }
    else
        {
        status = LoadConfig ( g_config );

        if ( SUCCESS != status )
            {
            return status;
            }

        pHelper->StartServices( g_config.console );
        }

    return SUCCESS;
    }

SINGLETON_DECLARE_STATICS(ServiceHelper)
