/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <BimFromDgnDb/BimFromDgnDb.h>

BEGIN_BIM_FROM_DGNDB_NAMESPACE

struct BimFromDgnDb
    {
    public:
        //! Converts a 1.6 dgndb/imodel to 2.0.  Calling application must have already initialized the host
        //! @param[in] inputPath    Full filename of the input 1.6 file
        //! @param[in] outputPath   Path to the directory where the 2.0 bim will be created
        BIM_FROM_DGNDB_EXPORT static bool Upgrade(WCharCP inputPath, WCharCP outputPath);
    };

END_BIM_FROM_DGNDB_NAMESPACE
