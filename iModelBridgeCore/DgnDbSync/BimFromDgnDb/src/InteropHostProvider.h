/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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