/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BimFromDgnDb/BimFromDgnDb.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "InteropHostProvider.h"

BEGIN_BIM_FROM_DGNDB_NAMESPACE

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

END_BIM_FROM_DGNDB_NAMESPACE

