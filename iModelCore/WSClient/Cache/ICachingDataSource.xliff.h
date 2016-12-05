/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/ICachingDataSource.xliff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/MobileDgnL10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
MOBILEDGN_TRANSLATABLE_STRINGS_START(ICachingDataSourceL10N, ICachingDataSource)
    {
    ERRORMESSAGE_InternalCache,             // =="Internal cache error."==
    ERRORMESSAGE_DataNotCached,             // =="Data not cached."==
    ERRORMESSAGE_FunctionalityNotSupported, // =="Requested functionality is not supported for this server version."==
    ERRORMESSAGE_SchemaError,               // =="Repository schema could not be updated. Please contact your server administrator"==
    };
MOBILEDGN_TRANSLATABLE_STRINGS_END

#define ICachingDataSourceLocalizedString(K) ICachingDataSourceL10N::GetString(ICachingDataSourceL10N::K, #K)
