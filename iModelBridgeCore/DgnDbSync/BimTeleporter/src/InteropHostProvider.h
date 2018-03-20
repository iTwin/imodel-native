/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/src/InteropHostProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BimTeleporter/BimTeleporter.h>

BEGIN_BIM_TELEPORTER_NAMESPACE

struct InteropHostProvider
{
public:
    static wchar_t const* GetTemporaryDirectory();
    static wchar_t const* GetAssetsDirectory();

};

END_BIM_TELEPORTER_NAMESPACE