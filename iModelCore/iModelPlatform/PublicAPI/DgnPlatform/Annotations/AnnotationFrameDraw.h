//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once


#include "AnnotationFrame.h"
#include "AnnotationFrameLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Used to draw an AnnotationFrame.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct AnnotationFrameDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationFrameLayoutCP m_frameLayout;
    Transform m_documentTransform;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFrameDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameDraw(AnnotationFrameLayoutCR);
    AnnotationFrameDraw(AnnotationFrameDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFrameDrawR operator=(AnnotationFrameDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFrameDrawPtr Create(AnnotationFrameLayoutCR frameLayout) { return new AnnotationFrameDraw(frameLayout); }
    AnnotationFrameDrawPtr Clone() const { return new AnnotationFrameDraw(*this); }
    AnnotationFrameLayoutCR GetFrameLayout() const { return *m_frameLayout; }
    TransformCR GetDocumentTransform() const { return m_documentTransform; }
    void SetDocumentTransform(TransformCR value) { m_documentTransform = value; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR) const;
};

END_BENTLEY_DGN_NAMESPACE
