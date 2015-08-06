//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameLayout.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationFrame.h"
#include "AnnotationTextBlockLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameLayout);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameLayout);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Computes size, geometry, and layout information for AnnotationFrame. This includes possible attachment points' IDs and computed physical points.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameLayout : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    bool m_isValid;
    AnnotationFrameCP m_frame;
    AnnotationTextBlockLayoutCP m_docLayout;
    double m_effectiveFontHeight;
    DRange2d m_contentRange;
    CurveVectorPtr m_frameGeometry;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFrameLayoutCR);
    void Invalidate() { m_isValid = false; }
    DGNPLATFORM_EXPORT void Update();

public:
    DGNPLATFORM_EXPORT AnnotationFrameLayout(AnnotationFrameCR, AnnotationTextBlockLayoutCR);
    AnnotationFrameLayout(AnnotationFrameLayoutCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFrameLayoutR operator=(AnnotationFrameLayoutCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFrameLayoutPtr Create(AnnotationFrameCR frame, AnnotationTextBlockLayoutCR docLayout) { return new AnnotationFrameLayout(frame, docLayout); }
    AnnotationFrameLayoutPtr Clone() const { return new AnnotationFrameLayout(*this); }

    AnnotationFrameCR GetFrame() const { return *m_frame; }
    AnnotationTextBlockLayoutCR GetDocumentLayout() const { return *m_docLayout; }
    double GetEffectiveFontHeight() const { const_cast<AnnotationFrameLayoutP>(this)->Update(); return m_effectiveFontHeight; }
    DRange2d GetContentRange() const { const_cast<AnnotationFrameLayoutP>(this)->Update(); return m_contentRange; }
    CurveVectorCR GetFrameGeometry() const { const_cast<AnnotationFrameLayoutP>(this)->Update(); return *m_frameGeometry; }

    DGNPLATFORM_EXPORT size_t GetAttachmentIdCount() const;
    DGNPLATFORM_EXPORT uint32_t GetAttachmentId(size_t) const;
    DGNPLATFORM_EXPORT void ComputePhysicalPointForAttachmentId(DPoint3dR, DVec3dR, uint32_t) const;
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
