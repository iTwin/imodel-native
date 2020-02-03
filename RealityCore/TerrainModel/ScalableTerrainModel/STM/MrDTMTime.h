/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/IMrDTMTime.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE


Time::TimeType              GetCTimeFor                        (const Time&                 time);
Time                        CreateTimeFrom                     (Time::TimeType              time);

Time                        CreateUnknownModificationTime      ();

Time                        GetFileLastModificationTimeFor     (const WChar*              filePath);




END_BENTLEY_MRDTM_NAMESPACE