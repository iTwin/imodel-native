/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshTime.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/01/19 20:04:51 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshTime.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


BENTLEY_SM_EXPORT Time::TimeType              GetCTimeFor                        (const Time&                 time);
BENTLEY_SM_EXPORT Time                        CreateTimeFrom                     (Time::TimeType              time);

BENTLEY_SM_EXPORT Time                        CreateUnknownModificationTime      ();

Time                        GetFileLastModificationTimeFor     (const WChar*              filePath);




END_BENTLEY_SCALABLEMESH_NAMESPACE
