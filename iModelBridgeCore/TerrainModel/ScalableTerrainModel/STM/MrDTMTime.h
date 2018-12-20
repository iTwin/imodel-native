/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMTime.h $
|    $RCSfile: MrDTMTime.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/01/19 20:04:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/IMrDTMTime.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE


Time::TimeType              GetCTimeFor                        (const Time&                 time);
Time                        CreateTimeFrom                     (Time::TimeType              time);

Time                        CreateUnknownModificationTime      ();

Time                        GetFileLastModificationTimeFor     (const WChar*              filePath);




END_BENTLEY_MRDTM_NAMESPACE