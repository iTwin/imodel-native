/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ClipVector.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "ClipPrimitive.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bvector<ClipPrimitivePtr> T_ClipPrimitiveVector;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  ClipVector : RefCounted<T_ClipPrimitiveVector>
{
private:
    ClipVector() {}
    ClipVector(ClipPrimitivePtr primitive);
    ClipVector(GPArrayCR gpa, double chordTolerance, double angleTolerance, double* zLow, double* zHigh, TransformCP transform);

public:
    DGNPLATFORM_EXPORT static ClipVectorPtr Create();
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateFromPrimitive(ClipPrimitivePtr primitive);
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateFromGPA(GPArrayCR gpa, double chordTolerance, double angleTolerance, double* zLow, double* zHigh, TransformCP transform);
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateFromCurveVector(CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double* zLow = NULL, double* zHigh = NULL);
    DGNPLATFORM_EXPORT static ClipVectorPtr CreateCopy(ClipVectorCR vector);

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
    DGNPLATFORM_EXPORT BentleyStatus ApplyCameraToPlanes(double focalLength);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
