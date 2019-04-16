/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley\Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::RotateCurve (RotMatrixCR rMatrix)
    {
    return bspcurv_rotateCurve (this, this, &rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::TransformCurve (TransformCR transform)
    {
    return bspcurv_transformCurve (this, this, &transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::TransformCurve4d (DMatrix4dCR transform4d)
    {
    return bspcurv_transformCurve4d (this, this, &transform4d);
    }

END_BENTLEY_GEOMETRY_NAMESPACE