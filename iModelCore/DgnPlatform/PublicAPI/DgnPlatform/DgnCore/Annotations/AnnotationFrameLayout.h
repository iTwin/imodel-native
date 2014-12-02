//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameLayout.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationFrame.h"
#include "AnnotationTextBlockLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameLayout);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameLayout);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameLayout : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    bool m_isValid;
    AnnotationFrameCP m_frame;
    AnnotationTextBlockLayoutCP m_docLayout;
    double m_effectiveFontHeight;
    DRange2d m_contentRange;
    CurveVectorPtr m_frameGeometry;

    void CopyFrom(AnnotationFrameLayoutCR);
    void Invalidate();
    void Update();

public:
    DGNPLATFORM_EXPORT AnnotationFrameLayout(AnnotationFrameCR, AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT AnnotationFrameLayout(AnnotationFrameLayoutCR);
    DGNPLATFORM_EXPORT AnnotationFrameLayoutR operator=(AnnotationFrameLayoutCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFrameLayoutPtr Create(AnnotationFrameCR, AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT AnnotationFrameLayoutPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationFrameCR GetFrame() const;
    DGNPLATFORM_EXPORT AnnotationTextBlockLayoutCR GetDocumentLayout() const;
    DGNPLATFORM_EXPORT double GetEffectiveFontHeight() const;
    DGNPLATFORM_EXPORT DRange2d GetContentRange() const;
    DGNPLATFORM_EXPORT CurveVectorCR GetFrameGeometry() const;

    DGNPLATFORM_EXPORT size_t GetAttachmentIdCount() const;
    DGNPLATFORM_EXPORT UInt32 GetAttachmentId(size_t) const;
    DGNPLATFORM_EXPORT void ComputePhysicalPointForAttachmentId(DPoint3dR, DVec3dR, UInt32) const;

}; // AnnotationFrameLayout

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
