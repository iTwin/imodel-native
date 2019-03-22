/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ClipVector.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "ClipPrimitive.h"

BEGIN_BENTLEY_DGN_NAMESPACE

typedef bvector<ClipPrimitivePtr> T_ClipPrimitiveVector;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClipVector : RefCounted<T_ClipPrimitiveVector>
{
    // scratch vector for use in clip steps -- prevent reallocation per geometry tested.
    bvector<DSegment1d> m_clipIntervals;
    DRange3d m_boundingRange = DRange3d::NullRange();

    ClipVector() {}
    ClipVector(ClipPrimitiveP primitive) {push_back(primitive);}
    
    virtual uint32_t _GetExcessiveRefCountThreshold() const override {return 100000;} 

    static ClipVectorPtr Create() {return new ClipVector();}
    static ClipVectorPtr CreateFromPrimitive(ClipPrimitiveP primitive) {return new ClipVector(primitive);}
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateFromCurveVector(CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double* zLow = NULL, double* zHigh = NULL);
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateCopy(ClipVectorCR vector);

    DGNPLATFORM_EXPORT ClipVectorPtr Clone(TransformCP trans) const;
    DGNPLATFORM_EXPORT bool PointInside(DPoint3dCR point, double onTolerance = 0.0) const;
    DGNPLATFORM_EXPORT BentleyStatus TransformInPlace(TransformCR transform);
    DGNPLATFORM_EXPORT void Append(ClipVectorCR clip);
    DGNPLATFORM_EXPORT void AppendCopy(ClipVectorCR clip);
    DGNPLATFORM_EXPORT StatusInt ClipPolyface(PolyfaceQueryCR polyface, struct PolyfaceQuery::IClipToPlaneSetOutput& output, bool triangulateOutput) const;
    DGNPLATFORM_EXPORT void SetInvisible(bool invisible);
    DGNPLATFORM_EXPORT void ExtractBoundaryLoops(int *nLoops, int nLoopPoints[], DPoint2dP loopPoints[], ClipMask* clipMaskP, double* zFrontP, double* zBackP, TransformP transformP, DPoint2dP pointBuffer, size_t nPoints) const;
    DGNPLATFORM_EXPORT bool GetRange(DRange3dR range, TransformCP transform) const;
    DGNPLATFORM_EXPORT static BentleyStatus AppendShape(ClipVectorPtr& clip, DPoint2dCP points, size_t nPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible = false);
    DGNPLATFORM_EXPORT static BentleyStatus AppendPlanes(ClipVectorPtr& clip, ClipPlaneSetCR planes, bool invisible = false);
    DGNPLATFORM_EXPORT void ParseClipPlanes();
    DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool ignoreMasks = false) const;
    DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyRangeContainment (DRange3dCR, bool ignoreMasks = false) const;


    // Treat each plane as a homogeneous row vector ax,ay,az,aw.
    // Multiply [ax,ay,az,aw]*matrix
    DGNPLATFORM_EXPORT BentleyStatus MultiplyPlanesTimesMatrix (DMatrix4dCR matrix);

    DGNPLATFORM_EXPORT bool IsAnyLineStringPointInside (DPoint3dCP points, size_t n, bool closed);
    DGNPLATFORM_EXPORT bool IsCompletelyContained(DPoint3dCP points, size_t n, bool closed);

    DGNPLATFORM_EXPORT bool IsAnyPointInside(DEllipse3dCR arc, bool closed);
    DGNPLATFORM_EXPORT bool IsCompletelyContained (DEllipse3dCR, bool closed);
    DGNPLATFORM_EXPORT bool IsAnyPointInside(MSBsplineCurveCR curve);
    DGNPLATFORM_EXPORT bool IsCompletelyContained(MSBsplineCurveCR curve);
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT static ClipVectorPtr FromJson(JsonValueCR);
};


END_BENTLEY_DGN_NAMESPACE
