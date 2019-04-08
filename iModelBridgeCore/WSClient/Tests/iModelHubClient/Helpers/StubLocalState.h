/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/StubLocalState.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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