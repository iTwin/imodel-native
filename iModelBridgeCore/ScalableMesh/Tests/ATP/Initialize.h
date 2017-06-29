#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


namespace ScalableMeshATPexe
{
void InitializeATP(DgnPlatformLib::Host& host);
void CloseATP();
};