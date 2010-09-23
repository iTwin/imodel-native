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
 
Bentley::NativeLogging::ILogger * ECObjectsLogger::s_log = NULL;  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::NativeLogging::ILogger * ECObjectsLogger::Log()
    {
    if (NULL == s_log)
        {
        s_log = LoggingManager::GetLogger(L"ECObjectsNative");
        }
    return s_log;
    }

END_BENTLEY_EC_NAMESPACE
