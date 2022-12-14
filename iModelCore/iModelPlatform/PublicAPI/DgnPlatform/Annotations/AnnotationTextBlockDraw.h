//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once


#include "AnnotationTextBlock.h"
#include "AnnotationTextBlockLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationTextBlockDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextBlockDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Used to draw an AnnotationTextBlock.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct AnnotationTextBlockDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationTextBlockLayoutCP m_layout;
    Transform m_documentTransform;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationTextBlockDrawCR);
    BentleyStatus DrawTextRun(AnnotationLayoutRunCR, AnnotationLayoutLineCR, Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR, TransformCR) const;
    BentleyStatus DrawFractionRun(AnnotationLayoutRunCR, AnnotationLayoutLineCR, Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR, TransformCR) const;
    BentleyStatus DrawLineBreakRun(AnnotationLayoutRunCR, AnnotationLayoutLineCR, Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR, TransformCR) const;

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlockDraw(AnnotationTextBlockLayoutCR);
    AnnotationTextBlockDraw(AnnotationTextBlockDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationTextBlockDrawR operator=(AnnotationTextBlockDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationTextBlockDrawPtr Create(AnnotationTextBlockLayoutCR layout) { return new AnnotationTextBlockDraw(layout); }
    AnnotationTextBlockDrawPtr Clone() const { return new AnnotationTextBlockDraw(*this); }
    AnnotationTextBlockLayoutCR GetLayout() const { return *m_layout; }
    TransformCR GetDocumentTransform() const { return m_documentTransform; }
    void SetDocumentTransform(TransformCR value) { m_documentTransform = value; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR) const;
};

END_BENTLEY_DGN_NAMESPACE
