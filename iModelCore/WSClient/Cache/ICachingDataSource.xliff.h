/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/ICachingDataSource.xliff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <DgnClientFx/DgnClientFxL10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
DGNCLIENTFX_TRANSLATABLE_STRINGS_START(ICachingDataSourceL10N, ICachingDataSource)
L10N_STRING(ERRORMESSAGE_InternalCacheError)            // =="Internal cache error"==
L10N_STRING(ERRORMESSAGE_DataNotCached)                 // =="Data not cached error"==
L10N_STRING(ERRORMESSAGE_FunctionalityNotSupported)     // =="Requested functionality is not supported for this server version"==
L10N_STRING(ERRORMESSAGE_RepositorySchemaError)         // =="Repository schema could not be used. Contact your server administrator"==
DGNCLIENTFX_TRANSLATABLE_STRINGS_END

#define ICachingDataSourceLocalizedString(K) ICachingDataSourceL10N::GetString(ICachingDataSourceL10N::K())
