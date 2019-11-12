/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>
#include <Vortex/VortexLicenseCode.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     3/2015
//----------------------------------------------------------------------------------------
void BePointCloudApi::Initialize()
    {
    if(!ptIsInitialized())
        ptInitialize(BentleyInternal_vortexLicCode);
    }

