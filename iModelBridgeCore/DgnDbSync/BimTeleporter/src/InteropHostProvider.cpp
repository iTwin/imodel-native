/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/InteropHostProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BimTeleporter/BimTeleporter.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "InteropHostProvider.h"

BEGIN_BIM_TELEPORTER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
wchar_t const* InteropHostProvider::GetTemporaryDirectory()
    {
    BeFileName tempDir;
    if (BentleyStatus::ERROR == T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempDir, nullptr))
        return nullptr;

    return tempDir.GetName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
wchar_t const* InteropHostProvider::GetAssetsDirectory()
    {
    return T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory().GetName();
    }

END_BIM_TELEPORTER_NAMESPACE

