/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeJsonCpp/BeJsonUtilities.h>

/*--------------------------------------------------------------------------------------+
* @bsiclass
* TODO: use RuntimeJsonLocalState only
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState
    {
    static RuntimeJsonLocalState* Instance();
    };