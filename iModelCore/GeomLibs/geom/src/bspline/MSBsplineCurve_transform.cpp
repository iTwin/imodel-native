/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::RotateCurve (RotMatrixCR rMatrix)
    {
    return bspcurv_rotateCurve (this, this, &rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::TransformCurve (TransformCR transform)
    {
    return bspcurv_transformCurve (this, this, &transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::TransformCurve4d (DMatrix4dCR transform4d)
    {
    return bspcurv_transformCurve4d (this, this, &transform4d);
    }

END_BENTLEY_GEOMETRY_NAMESPACE