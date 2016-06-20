/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/BePointCloudApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Vortex/VortexAPI.h>

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/ExportMacros.h>

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

//========================================================================================
// @bsiclass                                                        Eric.Paquet     3/2015
//========================================================================================
struct BePointCloudApi
    {
    public:
        BEPOINTCLOUD_EXPORT     static  void    Initialize();

    private:
    };

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
