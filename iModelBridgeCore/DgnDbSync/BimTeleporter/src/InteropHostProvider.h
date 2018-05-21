/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/InteropHostProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <BimTeleporter/BimTeleporter.h>

BEGIN_BIM_TELEPORTER_NAMESPACE

struct InteropHostProvider
{
public:
    static wchar_t const* GetTemporaryDirectory();
    static wchar_t const* GetAssetsDirectory();

};

END_BIM_TELEPORTER_NAMESPACE