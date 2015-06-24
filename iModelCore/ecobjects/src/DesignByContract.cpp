/*--------------------------------------------------------------------------------------+
|
|     $Source: src/DesignByContract.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/BeTest.h>

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

    if (1 == s_globalIgnoreCount)
        BentleyApi::BeTest::SetFailOnAssert (false);

#if !defined (BENTLEY_WINRT) 
    // WIP: calling putenv is nonportable and generally a bad idea
    // to handle calls to BeAssert as well
    putenv(const_cast<char*>("MS_IGNORE_ASSERTS=1"));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
AssertDisabler::~AssertDisabler ()
    {    
    AssertDisabler::s_globalIgnoreCount--;

    if (0 == AssertDisabler::s_globalIgnoreCount)
        BentleyApi::BeTest::SetFailOnAssert (true);

#if !defined (BENTLEY_WINRT)
    // WIP: calling putenv is nonportable and generally a bad idea
    putenv(const_cast<char*>("MS_IGNORE_ASSERTS="));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LogFailureMessage (WCharCP message, ...)
    {
    if (AssertDisabler::AreAssertsDisabled())
        return;
    va_list arguments;
    WString msg;
    va_start (arguments, message);              
    msg.VSprintf (message, arguments);
    va_end (arguments);
    LOG.warning(msg.c_str());
    }



  