/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DesignByContract.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

int AssertDisabler::s_globalIgnoreCount = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssertDisabler::AreAssertsDisabled () 
    {
    return s_globalIgnoreCount > 0; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
AssertDisabler::AssertDisabler ()
    {    
    AssertDisabler::s_globalIgnoreCount++;

#if !defined (BENTLEY_WINRT) 
    // WIP: calling putenv is nonportable and generally a bad idea
    // to handle calls to BeAssert as well
    putenv("MS_IGNORE_ASSERTS=1");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
AssertDisabler::~AssertDisabler ()
    {    
    AssertDisabler::s_globalIgnoreCount--;

#if !defined (BENTLEY_WINRT)
    // WIP: calling putenv is nonportable and generally a bad idea
    putenv("MS_IGNORE_ASSERTS=");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LogFailureMessage (WCharCP message, ...)
    {
    va_list arguments;
    WString msg;
    va_start (arguments, message);              
    WString::VSprintf (msg, message, arguments);
    LOG.warning(msg.c_str());
    }



  