/*--------------------------------------------------------------------------------------+
|
|     $Source: BePointCloudApi.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

#include <PointoolsVortexAPI_DLL/vortexLicense.c>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.h>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_ResultCodes.h>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.cpp>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     3/2015
//----------------------------------------------------------------------------------------
void BePointCloudApi::Initialize()
    {
    static bool s_loaded = false;
    if(!s_loaded)
        {
        s_loaded = LoadPointoolsDLL ("PointoolsVortexAPI.dll");
        }

    if(!ptIsInitialized())
        {
        ptInitialize(vortexLicCode);
        }
    }

