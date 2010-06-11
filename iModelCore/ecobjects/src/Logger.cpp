/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/Logger.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include <BsiLogging\log4cxx.h>
#include <BsiLogging\bsiloginterop.h>

USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_EC_NAMESPACE
   
Bentley::NativeLogging::ILogger * Logger::s_logger = NULL;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CheckProcessDirectory
(
wchar_t *filepath, 
DWORD bufferSize
)
    {
    bwstring dllPath = ECFileUtilities::GetDllPath();
    if (0 == dllPath.length())
        return ERROR;
        
    wchar_t executingDirectory[_MAX_DIR];
    wchar_t executingDrive[_MAX_DRIVE];
    _wsplitpath(dllPath.c_str(), executingDrive, executingDirectory, NULL, NULL);

    // Look for a file called "logging.config.xml" in the executing process's directory
    _wmakepath(filepath, executingDrive, executingDirectory, L"logging.config.xml", L"xml");
    if (0 == _waccess(filepath, 0))
        return SUCCESS;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring GetLogConfigurationFilename()
    {
    wchar_t filepath[_MAX_PATH];

    if ((0 != GetEnvironmentVariableW(L"BENTLEY_LOGGING_CONFIG", filepath, _MAX_PATH)) && (0 ==_waccess(filepath, 0)))
        {
        wprintf (L"ECObjects.dll configuring logging with %s (Set by BENTLEY_LOGGING_CONFIG environment variable.)\n", filepath);
        return filepath;
        }
    else if (SUCCESS == CheckProcessDirectory(filepath, sizeof(filepath)))
        {
        wprintf (L"ECObjects.dll configuring logging using %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
        return filepath;
        }
    else if (0 != GetEnvironmentVariableW(L"OutRoot", filepath, _MAX_PATH))
        {
        wchar_t * processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
        wcscat (filepath, processorArchitecture);
        wcscat (filepath, L"\\Product\\ECFrameworkNativeTest\\Tests\\logging.config.xml");
        
        if (0 ==_waccess(filepath, 0))
            {
            wprintf (L"ECObjects.dll configuring logging with %s. Override by setting BENTLEY_LOGGING_CONFIG in environment.\n", filepath);
            return filepath;
            }
        }

    return L"";
    }       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::NativeLogging::ILogger * Logger::GetLogger
(
void
)
    {
    if (NULL == s_logger)
        {
        // If the hosting application hasn't already established a logging context
        if ( !LoggerConfig::isInterfaceRegistered())
            {       
            wchar_t envValue[20];            
            if (0 < GetEnvironmentVariableW(L"BENTLEY_LOGGING_MANAGED", envValue, 20))
                {
                Provider::ManagedInteropProvider* pLog = new Provider::ManagedInteropProvider();
                LoggerConfig::registerLogInterface (pLog);
                }
            else
                {                    
                bwstring filepath = GetLogConfigurationFilename();
                if (filepath.size() > 0)
                    {   
                    Provider::Log4cxxLogger* pLog = new Provider::Log4cxxLogger ();
                    if (pLog != NULL)
                        {                             
                        pLog->LoadConfiguration (filepath.c_str());
                        LoggerConfig::registerLogInterface(pLog);  
                        }
                    }                      
                }
            }

        s_logger = LoggerRegistry::getLogger (L"ECObjectsNative");
        }

    return s_logger;
    }

END_BENTLEY_EC_NAMESPACE
