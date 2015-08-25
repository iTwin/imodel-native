/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshTime.h $
|    $RCSfile: ScalableMeshTime.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/01/19 20:04:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshTime.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


Time::TimeType              GetCTimeFor                        (const Time&                 time);
Time                        CreateTimeFrom                     (Time::TimeType              time);

Time                        CreateUnknownModificationTime      ();

Time                        GetFileLastModificationTimeFor     (const WChar*              filePath);




END_BENTLEY_SCALABLEMESH_NAMESPACE
