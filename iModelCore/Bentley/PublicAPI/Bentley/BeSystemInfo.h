/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeSystemInfo.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// __BENTLEY_INTERNAL_ONLY__

#include "NonCopyableClass.h"
#include "WString.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Static methods for retrieving system information.
// @bsiclass                                                    John.Gooding    01/2013
//=======================================================================================
struct BeSystemInfo :  NonCopyableClass
{
private:
    BeSystemInfo () {};

public:
#if defined (__APPLE__) || defined (ANDROID) || defined (BENTLEY_WIN32) || defined (__linux)
    BENTLEYDLL_EXPORT static uint64_t GetAmountOfPhysicalMemory ();
#endif

#if defined (__APPLE__)
    BENTLEYDLL_EXPORT static Utf8String GetMachineName();
    BENTLEYDLL_EXPORT static Utf8String GetModelName();
#endif

    BENTLEYDLL_EXPORT static uint32_t GetNumberOfCpus();
};

END_BENTLEY_NAMESPACE

