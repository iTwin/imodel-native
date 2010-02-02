/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/Logger.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

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
    wchar_t strExePath [MAX_PATH];
    if (0 == (GetModuleFileNameW (NULL, strExePath, MAX_PATH)))
        return ERROR;
        
    wchar_t executingDirectory[_MAX_DIR];
    wchar_t executingDrive[_MAX_DRIVE];
    _wsplitpath(strExePath, executingDrive, executingDirectory, NULL, NULL);

    // Look for a file called "log4cxx_properties.xml" in the executing process's directory
    _wmakepath(filepath, executingDrive, executingDirectory, L"log4cxx_properties", L"xml");
    if (0 == _waccess(filepath, 0))
        return SUCCESS;
    return ERROR;
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
        if (false == LoggerConfig::isInterfaceRegistered())
            {
            Provider::Log4cxxLogger* pLog = new Provider::Log4cxxLogger;
            wchar_t filepath[_MAX_PATH];

            if ((0 != GetEnvironmentVariableW(L"ECOBJECTS_LOGGING_CONFIG", filepath, _MAX_PATH)) && (0 ==_waccess(filepath, 0)))
                pLog->LoadConfiguration (filepath);
            else if (SUCCESS == CheckProcessDirectory(filepath, sizeof(filepath)))
                pLog->LoadConfiguration(filepath);
            else
                {
                pLog->BasicConfiguration ();
                pLog->SetSeverity(L"ECObjectsNative", LOG_WARNING);
                }

            LoggerConfig::registerLogInterface(pLog);
            }

        s_logger = LoggerRegistry::getLogger (L"ECObjectsNative");
        }

    return s_logger;
    }

END_BENTLEY_EC_NAMESPACE
