//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

#include "stdafx.h"
#include "TimeStampUtils.h"

//---------------------------------------------------------------------------
// DESC: 
// HIST: Original - tigger - 6/99
// MISC: 
//---------------------------------------------------------------------------
wchar_t *aecTimeStamp_getUserName
    (
    wchar_t *szUserName           // <- Username - may be NULL
    )
    {
    static wchar_t szName[USER_NAME_SIZE];
    unsigned long szLen = USER_NAME_SIZE;

    if( wcslen( szName ) == 0 )
        GetUserNameW(szName, &szLen);

    if (szLen <= 0)
        {
        // Don't really see how this could happen.  But just in case.
        // How could you be logged on with a 0 length username?
        szName[0] = '*';
        szName[1] = '\0';
        szLen = 1;
        }

    // We never want to return more that 31 characters + 1 NULL terminator.
    if (szLen >= USER_NAME_SIZE)
        {
        szLen = USER_NAME_SIZE - 1;
        szName[USER_NAME_SIZE-1] = '\0';
        }

    if (szUserName)
        wcsncpy(szUserName, szName, szLen);

    return(szName);
    }

//---------------------------------------------------------------------------
// DESC: 
// HIST: Original - tigger - 6/99
// MISC: 
//---------------------------------------------------------------------------
int aecTimeStamp_getStamp
    (
    SYSTEMTIME *pSystemTime     // <- time stamp structure
    )
    {
    int sts = SUCCESS;

    if (pSystemTime)
        {
        memset(pSystemTime, 0, sizeof(SYSTEMTIME));
        GetLocalTime(pSystemTime);
        }
    else
        {
        sts = ERROR;
        }

    return(sts);
    }
