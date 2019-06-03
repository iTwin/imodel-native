#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


namespace ScalableMeshATPexe
{
void InitializeATP(DgnPlatformLib::Host& host);
void CloseATP();
};