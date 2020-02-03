/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformLib.h>
#include "SMWorkerDefinitions.h"

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif


BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE

void InitializeWorker(DgnPlatformLib::Host& host);

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE