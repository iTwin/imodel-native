/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/DataSourceCache.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeSQLite/L10N.h>

//=======================================================================================
// @bsiclass
//=======================================================================================
BENTLEY_TRANSLATABLE_STRINGS_START(DataSourceCacheL10N, DataSourceCache)
L10N_STRING(ERROR_FileIsLocked)     // =="File '%s' is currently open in another application. Close the file and try again."==
BENTLEY_TRANSLATABLE_STRINGS_END

#define DataSourceCacheLocalizedString(K) DataSourceCacheL10N::GetString(DataSourceCacheL10N::K())
