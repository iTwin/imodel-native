/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnTableElementScanner.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/ScanCriteria.h>


BEGIN_DGNV8_NAMESPACE

//=======================================================================================
//! Access to tables
//! @bsiclass                                                   Sam.Wilson      10/2008
//=======================================================================================
struct          DgnTableElementScanner
{

//! @bsimethod                                    Sam.Wilson                      10/2008
struct  ScanCallback
    {
    //! @param tableEh    A table (header) element on the requested level.
    //! @return non-zero to stop the scan or zero to continue scanning
    //! @bsimethod                                    Sam.Wilson                      10/2008
    virtual StatusInt OnTableElement (ElementHandleCR tableEh, ScanCriteria*) = 0;
    virtual ~ScanCallback() {}
    };

//! Process tables in cache.
//! @param[in]      cb                  Callback to process tables found.
//! @param[in]      cache               The cache to query for tables.
//! @param[in]      model               Optional modelref to pass to callback.
//! @param[in]      tableLevelIn        Optional table level.
//! @param[in]      forceNonModelCache  If true, scan the dictionary model of the file; else, scan the specified cache.
//! @bsimethod                                    Sam.Wilson                      10/2008
static StatusInt ScanTableElements (ScanCallback& cb, DgnModelR cache, DgnModelP model, int tableLevelIn, bool forceNonModelCache = true);

};

END_DGNV8_NAMESPACE
