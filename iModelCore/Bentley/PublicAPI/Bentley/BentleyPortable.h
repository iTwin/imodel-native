/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BentleyPortable.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

// This header file is for the use of client code that uses Bentley.dll

#ifdef BENTLEY_H
    #error Include this file first, before any other Bentley or DgnPlatform header file, if you want to make your code able to include Bentley portability headers.
#endif

#include "Bentley.h"

#define BENTLEY_PORTABLE_H_

