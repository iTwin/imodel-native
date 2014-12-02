//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderLayout.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderLayout : public RefCountedBase
{
//__PUBLISH_SECTION_END__
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

    void CopyFrom(AnnotationLeaderLayoutCR);
    void Invalidate();
    void Update();

public:
    DGNPLATFORM_EXPORT AnnotationLeaderLayout(AnnotationLeaderCR, AnnotationFrameLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderLayout(AnnotationLeaderLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderLayoutR operator=(AnnotationLeaderLayoutCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderLayoutPtr Create(AnnotationLeaderCR, AnnotationFrameLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderLayoutPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationLeaderCR GetLeader() const;
    DGNPLATFORM_EXPORT AnnotationFrameLayoutCR GetFrameLayout() const;
    DGNPLATFORM_EXPORT TransformCR GetFrameTransform() const;
    DGNPLATFORM_EXPORT void SetFrameTransform(TransformCR);
    DGNPLATFORM_EXPORT DPoint3dCR GetSourcePhysicalPoint() const;
    DGNPLATFORM_EXPORT DVec3dCR GetSourceTangent() const;
    DGNPLATFORM_EXPORT DPoint3dCR GetTargetPhysicalPoint() const;
    DGNPLATFORM_EXPORT CurveVectorCR GetLineGeometry() const;
    DGNPLATFORM_EXPORT CurveVectorCR GetTerminatorGeometry() const;
    DGNPLATFORM_EXPORT TransformCR GetTerminatorTransform() const;

}; // AnnotationLeaderLayout

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
