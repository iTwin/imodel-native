//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationLeaderLayout.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationLeader.h"
#include "AnnotationFrameLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeaderLayout);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderLayout);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Computes size, geometry, and layout information for AnnotationLeader.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderLayout : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    bool m_isValid;
    AnnotationLeaderCP m_leader;
    AnnotationFrameLayoutCP m_frameLayout;
    Transform m_frameTransform;
    DPoint3d m_sourcePhysicalPoint;
    DVec3d m_sourceTangent;
    DPoint3d m_targetPhysicalPoint;
    CurveVectorPtr m_lineGeometry;
    CurveVectorPtr m_terminatorGeometry;
    Transform m_terminatorTransform;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLeaderLayoutCR);
    void Invalidate() { m_isValid = false; }
    DGNPLATFORM_EXPORT void Update();

public:
    DGNPLATFORM_EXPORT AnnotationLeaderLayout(AnnotationLeaderCR, AnnotationFrameLayoutCR);
    AnnotationLeaderLayout(AnnotationLeaderLayoutCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLeaderLayoutR operator=(AnnotationLeaderLayoutCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLeaderLayoutPtr Create(AnnotationLeaderCR leader, AnnotationFrameLayoutCR frameLayout) { return new AnnotationLeaderLayout(leader, frameLayout); }
    AnnotationLeaderLayoutPtr Clone() const { return new AnnotationLeaderLayout(*this); }

    AnnotationLeaderCR GetLeader() const { return *m_leader; }
    AnnotationFrameLayoutCR GetFrameLayout() const { return *m_frameLayout; }
    TransformCR GetFrameTransform() const { return m_frameTransform; }
    void SetFrameTransform(TransformCR value) { m_frameTransform = value; }
    DPoint3dCR GetSourcePhysicalPoint() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return m_sourcePhysicalPoint; }
    DVec3dCR GetSourceTangent() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return m_sourceTangent; }
    DPoint3dCR GetTargetPhysicalPoint() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return m_targetPhysicalPoint; }
    CurveVectorCR GetLineGeometry() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return *m_lineGeometry; }
    TransformCR GetTerminatorTransform() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return m_terminatorTransform; }
    CurveVectorCR GetTerminatorGeometry() const { const_cast<AnnotationLeaderLayoutP>(this)->Update(); return *m_terminatorGeometry; }
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
