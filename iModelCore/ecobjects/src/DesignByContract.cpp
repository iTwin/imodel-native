/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/DesignByContract.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
AssertDisabler::~AssertDisabler ()
    {    
    AssertDisabler::s_globalIgnoreCount--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LogFailureMessage (const wchar_t * message, ...)
    {
    va_list arguments;
    va_start (arguments, message);              
    vwprintf (message, arguments);    
    }



  