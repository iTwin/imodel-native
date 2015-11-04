/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ClipPrimitive.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include <Geom/ClipPlaneSet.h>

BEGIN_BENTLEY_DGN_NAMESPACE


typedef bvector<DPoint2d>       T_ClipPolygon;

/*=================================================================================**//**
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct ClipPolygon : bvector <DPoint2d>
{
    ClipPolygon () { }
    ClipPolygon (DPoint2dCP points, size_t nPoints);
    void Init (DPoint2dCP points, size_t nPoints);

};  // ClipPolygon


typedef ClipPolygon const*      ClipPolygonCP;
typedef ClipPolygon const&      ClipPolygonCR;

/*=================================================================================**//**
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct  ClipPrimitive  : RefCountedBase
{
protected:
    virtual     ClipPlaneSetCP   _GetClipPlanes () const = 0;
    virtual     ClipPlaneSetCP   _GetMaskPlanes () const = 0;
    virtual     ClipPrimitive*   _Clone () const = 0;
    virtual     bool             _GetInvisible () const = 0;

    virtual     GPArrayCP        _GetGPA (bool onlyIfNonLinear) const           { return NULL; }
    virtual     MSBsplineCurveCP _GetBCurve ()            const                 { return NULL; }
    virtual     double           _GetZLow ()              const                 { return 0.0; }
    virtual     double           _GetZHigh ()             const                 { return 0.0; }
    virtual     bool             _ClipZLow ()             const                 { return false; }
    virtual     bool             _ClipZHigh ()            const                 { return false; }
    virtual     bool             _IsMask ()               const                 { return false; }
    virtual     ClipPolygonCP    _GetPolygon ()           const                 { return NULL; }
    virtual     TransformCP      _GetTransformToClip ()   const                 { return NULL; }
    virtual     TransformCP      _GetTransformFromClip () const                 { return NULL; }

    virtual     void             _SetIsMask (bool isMask);
    virtual     void             _SetZLow (double zLow); 
    virtual     void             _SetZHigh (double zHigh);
    virtual     void             _SetClipZLow (bool clipZLow);
    virtual     void             _SetClipZHigh (bool clipZHigh);
    virtual     void             _SetInvisible (bool invisible) = 0;
    virtual     BentleyStatus    _TransformInPlace (TransformCR transform);
    virtual     bool             _GetRange (DRange3dR range, TransformCP transform, bool returnMaskRange) const = 0;
    virtual     BentleyStatus    _ApplyCameraToPlanes (double focalLength) { return ERROR; }



public:     
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateCopy (ClipPrimitiveCR primitive); 
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromBlock (DPoint3dCR low, DPoint3dCR high, bool outside, ClipMask clipMask, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromShape (DPoint2dCP points, size_t numPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromClipPlanes (ClipPlaneSetCR planes, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromGPA (GPArrayCR gpa, double chordTolerance, double angleTolerance, bool isMask, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromGPA (GPArrayCR gpa, ClipPolygonCR, bool isMask, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static ClipPrimitivePtr  CreateFromBoundaryCurveVector (CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);

    DGNPLATFORM_EXPORT ClipPlaneSetCP           GetClipPlanes () const;
    DGNPLATFORM_EXPORT ClipPlaneSetCP           GetMaskPlanes () const;
    DGNPLATFORM_EXPORT ClipPlaneSetCP           GetMaskOrClipPlanes () const;
    DGNPLATFORM_EXPORT GPArrayCP                GetGPA (bool onlyIfNonLinear) const;
    DGNPLATFORM_EXPORT MSBsplineCurveCP         GetBCurve () const;
    DGNPLATFORM_EXPORT bool                     GetRange (DRange3dR range, TransformCP transform, bool returnMaskRange = false) const;
    DGNPLATFORM_EXPORT TransformCP              GetTransformToClip () const;                                   //! Get Transform from world to clip.
    DGNPLATFORM_EXPORT TransformCP              GetTransformFromClip () const;                                 //! Get Transform from clip to world.
    DGNPLATFORM_EXPORT bool                     GetInvisible () const;
    DGNPLATFORM_EXPORT bool                     GetTransforms (TransformP forward, TransformP inverse);
    DGNPLATFORM_EXPORT BentleyStatus            TransformInPlace (TransformCR transform);
    DGNPLATFORM_EXPORT bool                     IsMask () const;
    DGNPLATFORM_EXPORT bool                     ClipZHigh () const;
    DGNPLATFORM_EXPORT bool                     ClipZLow () const;
    DGNPLATFORM_EXPORT double                   GetZLow () const;
    DGNPLATFORM_EXPORT double                   GetZHigh () const;
    DGNPLATFORM_EXPORT ClipPolygonCP            GetPolygon () const;
    DGNPLATFORM_EXPORT BentleyStatus            GetTestPoint (DPoint3dR point);
    DGNPLATFORM_EXPORT bool                     PointInside (DPoint3dCR point, double onTolerance = 0.0) const;
    DGNPLATFORM_EXPORT void                     SetIsMask (bool isMask);
    DGNPLATFORM_EXPORT void                     SetZLow (double zLow);
    DGNPLATFORM_EXPORT void                     SetZHigh (double zHigh);
    DGNPLATFORM_EXPORT void                     SetClipZLow (bool clipZLow);
    DGNPLATFORM_EXPORT void                     SetClipZHigh (bool clipZHigh);
    DGNPLATFORM_EXPORT void                     SetInvisible (bool invisible); 
    DGNPLATFORM_EXPORT bool                     IsXYPolygon () const;
    DGNPLATFORM_EXPORT bool                     ContainsZClip() const;
    DGNPLATFORM_EXPORT void                     ParseClipPlanes () const;
    DGNPLATFORM_EXPORT BentleyStatus            ApplyCameraToPlanes (double focalLength);
    DGNPLATFORM_EXPORT ClipPlaneContainment     ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool ignoreMasks = false) const;
    DGNPLATFORM_EXPORT void                     TransformToClip (DPoint3dR point) const;
    DGNPLATFORM_EXPORT void                     TransformFromClip (DPoint3dR point) const;


};


END_BENTLEY_DGN_NAMESPACE
