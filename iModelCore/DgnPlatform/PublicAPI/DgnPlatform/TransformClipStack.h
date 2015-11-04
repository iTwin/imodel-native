/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TransformClipStack.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "ClipVector.h"
#include <stack>
#include <Bentley/bvector.h>
#include "IViewDraw.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley      02/07
+===============+===============+===============+===============+===============+======*/
struct TransformClipStack
{
public:

~TransformClipStack();

DGNPLATFORM_EXPORT bool IsEmpty () const;
DGNPLATFORM_EXPORT void Clear   ();
DGNPLATFORM_EXPORT void PushTransform (TransformCR trans);
DGNPLATFORM_EXPORT void PushClip (ClipVectorCR clip);
DGNPLATFORM_EXPORT void PushClipPlanes (ClipPlaneCP planes, size_t nPlanes);
DGNPLATFORM_EXPORT void PushClipPlaneSets (ClipPlaneSetCR planes);
DGNPLATFORM_EXPORT void PushIdentity ();
DGNPLATFORM_EXPORT void Pop (ViewContextR viewContext);
DGNPLATFORM_EXPORT void PopAll (ViewContextR viewContext);
DGNPLATFORM_EXPORT void SetViewIndependent ();
DGNPLATFORM_EXPORT void IncrementPushedToDrawGeom ();
DGNPLATFORM_EXPORT bool IsViewIndependent () const;
DGNPLATFORM_EXPORT bool TestRay (DPoint3dCR point, DVec3dCR direction) const;
DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyRange (DRange3dCR range, bool ignoreMasks = false) const;
DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyElementRange (DRange3dCR range, bool is3d, bool ignoreMasks = false) const;
DGNPLATFORM_EXPORT ClipPlaneContainment ClassifyPoints (DPoint3dCP points, size_t nPoints, bool ignoreMasks = false) const;
DGNPLATFORM_EXPORT bool GetRayIntersection (double& intersectDistance, DPoint3dCR point, DVec3dCR direction) const;
DGNPLATFORM_EXPORT bool TestPoint (DPoint3dCR point) const;
DGNPLATFORM_EXPORT ClipVectorCP GetClip () const;
DGNPLATFORM_EXPORT ClipVectorCP GetDrawGeomClip () const;
DGNPLATFORM_EXPORT BentleyStatus GetInverseTransform (TransformR transform) const;
DGNPLATFORM_EXPORT BentleyStatus GetTransform (TransformR transform) const;
DGNPLATFORM_EXPORT BentleyStatus GetTransformFromIndex (TransformR transform, size_t index) const;
DGNPLATFORM_EXPORT BentleyStatus GetTransformFromTopToIndex (TransformR transform, size_t index) const;
DGNPLATFORM_EXPORT TransformCP GetTransformCP() const;
DGNPLATFORM_EXPORT double GetTransformScale () const;
DGNPLATFORM_EXPORT size_t GetSize () const;

private:

typedef bvector<struct TransformClip*> T_TransformClips;

T_TransformClips    m_transformClips;

TransformClip* GetTop ();

}; // TransformClipStack

END_BENTLEY_DGNPLATFORM_NAMESPACE
