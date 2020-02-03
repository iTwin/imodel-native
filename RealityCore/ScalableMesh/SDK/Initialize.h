/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformLib.h>

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


namespace ScalableMeshSDKexe
    {
    void InitializeSDK(BeFileName& systemDtyPath, BeFileName& customDtyPath);
    void CloseSDK();
    };