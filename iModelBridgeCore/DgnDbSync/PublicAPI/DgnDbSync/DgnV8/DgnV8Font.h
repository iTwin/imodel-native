/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnDbSync/DgnDbSync.h>
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnV8FontPersistence : NonCopyableClass
{
    struct File
    {
        DGNDBSYNC_EXPORT static T_DgnFontPtrs FromRscFile(BentleyApi::BeFileNameCR, BentleyApi::Utf8CP nameFilter);
    };
};

END_BENTLEY_DGN_NAMESPACE
