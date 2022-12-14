/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/BeTest.h>

int AssertDisabler::s_globalIgnoreCount = 0;
PUSH_DISABLE_DEPRECATION_WARNINGS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AssertDisabler::AreAssertsDisabled ()
    {
    return s_globalIgnoreCount > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
END_BENTLEY_ECOBJECT_NAMESPACE
POP_DISABLE_DEPRECATION_WARNINGS
