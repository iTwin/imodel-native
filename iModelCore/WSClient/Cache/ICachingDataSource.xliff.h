/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <BeSQLite/L10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(ICachingDataSourceL10N, ICachingDataSource)
L10N_STRING(ERRORMESSAGE_InternalCache)                 // =="Internal cache error"==
L10N_STRING(ERRORMESSAGE_DataNotCached)                 // =="Data not cached error"==
L10N_STRING(ERRORMESSAGE_FunctionalityNotSupported)     // =="Requested functionality is not supported for this server version"==
L10N_STRING(ERRORMESSAGE_SchemaError)                   // =="Repository schema could not be updated. Please contact your server administrator"==
BENTLEY_TRANSLATABLE_STRINGS_END

#define ICachingDataSourceLocalizedString(K) ICachingDataSourceL10N::GetString(ICachingDataSourceL10N::K())
