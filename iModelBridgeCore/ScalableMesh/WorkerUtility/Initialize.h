#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


namespace ScalableMeshWorker
    {
    void InitializeWorker(DgnPlatformLib::Host& host);
    };