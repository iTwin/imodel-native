/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GeomHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

struct GeomHelper{

public:
    static const double PLANE_LEN;
    static CurveVectorPtr computeShape(double len = PLANE_LEN);
    static CurveVectorPtr computeShape2d(double len = PLANE_LEN);
    static MSBsplineSurfacePtr CreateGridSurface(DPoint3dCR origin, double dx, double dy, size_t order, size_t numX, size_t numY);
    static TextStringPtr CreateTextString(TextStringStylePtr style=NULL);

};
