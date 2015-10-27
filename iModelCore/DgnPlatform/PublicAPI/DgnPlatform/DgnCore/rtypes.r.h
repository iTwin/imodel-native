/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/rtypes.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

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
    uint32_t numChars;
    char    firstChar[1]; // WIP_CHAR_OK - Persistence

    } CharString;

#endif // defined (resource)
