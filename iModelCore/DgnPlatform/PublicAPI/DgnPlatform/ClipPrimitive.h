/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ClipPrimitive.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <Geom/ClipPlaneSet.h>

BEGIN_BENTLEY_DGN_NAMESPACE

typedef bvector<DPoint2d>  T_ClipPolygon;

/*=================================================================================**//**
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct ClipPolygon : bvector<DPoint2d>
{
    ClipPolygon() {}
    ClipPolygon(DPoint2dCP points, size_t nPoints);
    void Init(DPoint2dCP points, size_t nPoints);
};


typedef ClipPolygon const* ClipPolygonCP;
typedef ClipPolygon const& ClipPolygonCR;

/*=================================================================================**//**
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct  ClipPrimitive  : RefCountedBase
{
protected:
    virtual ClipPlaneSetCP _GetClipPlanes() const = 0;
    virtual ClipPlaneSetCP _GetMaskPlanes() const = 0;
    virtual ClipPrimitive* _Clone() const = 0;
    virtual bool _GetInvisible() const = 0;

    virtual GPArrayCP _GetGPA (bool onlyIfNonLinear) const { return NULL; }
    virtual MSBsplineCurveCP _GetBCurve() const { return NULL; }
    virtual double _GetZLow() const { return 0.0; }
    virtual double _GetZHigh() const { return 0.0; }
    virtual bool _ClipZLow() const { return false; }
    virtual bool _ClipZHigh() const { return false; }
    virtual bool _IsMask() const { return false; }
    virtual ClipPolygonCP _GetPolygon() const { return NULL; }
    virtual TransformCP _GetTransformToClip() const { return NULL; }
    virtual TransformCP _GetTransformFromClip() const { return NULL; }

    virtual void _SetInvisible(bool invisible) = 0;
    virtual bool _GetRange(DRange3dR range, TransformCP transform, bool returnMaskRange) const = 0;
    virtual BentleyStatus _ApplyCameraToPlanes(double focalLength) { return ERROR; }
    virtual void _SetIsMask(bool isMask) { BeAssert(false); }
    virtual void _SetZLow(double zLow) { BeAssert(false); } 
    virtual void _SetClipZLow(bool clipZLow) { BeAssert(!clipZLow); } 
    virtual void _SetZHigh(double zHigh) { BeAssert(false); }
    virtual void _SetClipZHigh(bool clipZHigh) { BeAssert(!clipZHigh); }
    virtual BentleyStatus _TransformInPlace(TransformCR transform) { BeAssert(false); return ERROR; }

public:     
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateCopy(ClipPrimitiveCR primitive); 
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromBlock(DPoint3dCR low, DPoint3dCR high, bool outside, ClipMask clipMask, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromShape(DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromClipPlanes(ClipPlaneSetCR planes, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromGPA(GPArrayCR gpa, double chordTolerance, double angleTolerance, bool isMask, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromGPA(GPArrayCR gpa, ClipPolygonCR, bool isMask, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr CreateFromBoundaryCurveVector(CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);

    ClipPlaneSetCP GetClipPlanes() const { return _GetClipPlanes(); }
    ClipPlaneSetCP GetMaskPlanes() const { return _GetMaskPlanes(); }
    ClipPlaneSetCP GetMaskOrClipPlanes() const { return NULL == _GetMaskPlanes() ? _GetClipPlanes() : _GetMaskPlanes(); }

    GPArrayCP GetGPA(bool onlyIfNonLinear) const { return _GetGPA(onlyIfNonLinear); }
    MSBsplineCurveCP GetBCurve() const { return _GetBCurve(); }
    bool IsMask() const { return _IsMask(); }
    bool ClipZLow() const { return _ClipZLow(); }
    bool ClipZHigh() const { return _ClipZHigh(); }
    double GetZLow() const { return _GetZLow(); }
    double GetZHigh() const { return _GetZHigh(); }
    ClipPolygonCP GetPolygon() const { return _GetPolygon(); }
    TransformCP GetTransformToClip() const { return _GetTransformToClip(); } 
    TransformCP GetTransformFromClip() const { return _GetTransformFromClip(); } 
    bool GetInvisible() const { return _GetInvisible(); }
    bool GetRange(DRange3dR range, TransformCP transform, bool returnMaskRange=false) const { return _GetRange(range, transform, returnMaskRange); }
    void SetIsMask(bool isMask) { _SetIsMask(isMask); }
    void SetZLow(double zLow) { _SetZLow(zLow); }
    void SetZHigh(double zHigh) { _SetZHigh(zHigh); }
    void SetClipZLow(bool clipZLow) { _SetClipZLow(clipZLow); }
    void SetClipZHigh(bool clipZHigh) { _SetClipZHigh(clipZHigh); }
    void SetInvisible(bool invisible) { _SetInvisible(invisible); }
    BentleyStatus TransformInPlace(TransformCR transform) { return _TransformInPlace(transform); }
    BentleyStatus ApplyCameraToPlanes(double focalLength) { return _ApplyCameraToPlanes(focalLength); }
    void ParseClipPlanes() const { GetClipPlanes(); GetMaskPlanes(); }

    DGNPLATFORM_EXPORT bool GetTransforms(TransformP forward, TransformP inverse);
    DGNPLATFORM_EXPORT BentleyStatus GetTestPoint(DPoint3dR point);
    DGNPLATFORM_EXPORT bool PointInside(DPoint3dCR point, double onTolerance = 0.0) const;
    DGNPLATFORM_EXPORT bool IsXYPolygon() const;
    DGNPLATFORM_EXPORT bool ContainsZClip() const;
    DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyPointContainment(DPoint3dCP points, size_t nPoints, bool ignoreMasks = false) const;
    DGNPLATFORM_EXPORT void TransformToClip(DPoint3dR point) const;
    DGNPLATFORM_EXPORT void TransformFromClip(DPoint3dR point) const;
};

END_BENTLEY_DGN_NAMESPACE
