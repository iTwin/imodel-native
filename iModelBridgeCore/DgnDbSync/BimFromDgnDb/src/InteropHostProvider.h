/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/src/InteropHostProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <BimFromDgnDb/BimFromDgnDb.h>

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct InteropHostProvider
{
public:
    static wchar_t const* GetTemporaryDirectory();
    static wchar_t const* GetAssetsDirectory();

};

END_BIM_FROM_DGNDB_NAMESPACE