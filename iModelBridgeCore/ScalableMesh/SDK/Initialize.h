#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

namespace ScalableMeshSDKexe
    {
    void InitializeSDK(BeFileName& systemDtyPath, BeFileName& customDtyPath);
    void CloseSDK();
    };