/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/rtypes.r.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <RmgrTools/Tools/rtypes.r.h>

/*----------------------------------------------------------------------+
|                                                                       |
|   Character String Resource Definition                                |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (resource)

typedef char CharString []; // WIP_CHAR_OK - Persistence
resourceclass CharString RTYPE_CharString;

#else // defined (resource)

typedef struct charstring
    {
    UInt32  numChars;
    char    firstChar[1]; // WIP_CHAR_OK - Persistence

    } CharString;

#endif // defined (resource)
