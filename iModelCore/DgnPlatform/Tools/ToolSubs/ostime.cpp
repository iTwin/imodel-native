/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <Bentley/BeTimeUtilities.h>
#include    <DgnPlatform/Tools/ostime.fdf>

/*----------------------------------------------------------------------+
|                                                                       |
| name          osTime_getCurrentMillis                                 |
|                                                                       |
| author        BarryBentley                            07/00           |
|                                                                       |
+----------------------------------------------------------------------*/
Public double   osTime_getCurrentMillis
(
void
)
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble ();
    }
