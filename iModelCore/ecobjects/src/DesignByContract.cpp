/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/DesignByContract.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
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
    int len;
    wchar_t *buffer;
    va_start (arguments, message);              

       len = _vscwprintf( message, arguments ) // _vscprintf doesn't count
                                + 1; // terminating '\0'
    
    buffer = (wchar_t*)malloc( len * sizeof(wchar_t) );

    vswprintf( buffer, message, arguments ); // C4996
    Bentley::EC::Logger::GetLogger()->warning(buffer );
    free( buffer );
    }



  