/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/ostime.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
