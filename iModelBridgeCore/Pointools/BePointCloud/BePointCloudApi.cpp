/*--------------------------------------------------------------------------------------+
|
|     $Source: BePointCloudApi.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

