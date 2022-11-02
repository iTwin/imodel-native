/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// RefCountedMSBspline support ...
// 1) Clear bits in bspline in constructor.
// 2) ReleaseMem in destructor
// 3) Allocate in MSBsplineCurve::CreatePtr ()
RefCountedMSBsplineCurve::RefCountedMSBsplineCurve ()    {Zero ();}
RefCountedMSBsplineCurve::~RefCountedMSBsplineCurve ()   {ReleaseMem ();}
MSBsplineCurvePtr MSBsplineCurve::CreatePtr ()           {return new RefCountedMSBsplineCurve ();}

RefCountedMSInterpolationCurve::RefCountedMSInterpolationCurve ()    {Zero ();}
RefCountedMSInterpolationCurve::~RefCountedMSInterpolationCurve ()   {ReleaseMem ();}
MSInterpolationCurvePtr MSInterpolationCurve::CreatePtr ()           {return new RefCountedMSInterpolationCurve ();}

RefCountedMSBsplineSurface::RefCountedMSBsplineSurface ()    {Zero ();}
RefCountedMSBsplineSurface::~RefCountedMSBsplineSurface ()   {ReleaseMem ();}
MSBsplineSurfacePtr MSBsplineSurface::CreatePtr ()           {return new RefCountedMSBsplineSurface ();}

MSBsplineSurfacePtr MSBsplineSurface::CreateCopyTransformed (TransformCR transform) const
    {
    MSBsplineSurfacePtr result = MSBsplineSurface::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspsurf_transformSurface (result.get (), this, &transform);
    return result;
    }

MSBsplineSurfacePtr MSBsplineSurface::Clone () const
    {
    MSBsplineSurfacePtr result = MSBsplineSurface::CreatePtr ();
    if (!result.IsValid ())
        return result;
    bspsurf_copySurface (result.get (), this);
    return result;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
