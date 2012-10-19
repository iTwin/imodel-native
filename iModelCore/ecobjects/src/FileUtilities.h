/*--------------------------------------------------------------------------------------+
|
|     $Source: src/FileUtilities.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE

struct ECFileUtilities
    {
private:
    static WString s_dllPath;
    ECFileUtilities(void) {}
    
public:
    static WString GetDllPath();
    };

END_BENTLEY_EC_NAMESPACE