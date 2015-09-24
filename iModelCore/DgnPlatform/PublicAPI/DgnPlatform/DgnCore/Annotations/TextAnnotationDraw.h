//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "TextAnnotation.h"
#include "../ElementGraphics.h"

DGNPLATFORM_TYPEDEFS(TextAnnotationDraw);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationDraw);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Used to draw a TextAnnotation.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct TextAnnotationDraw : public RefCountedBase
{
    //=======================================================================================
    //! Allows for the ViewContext-based TextAnnotationDraw to be used with an ElementGeometryBuilder.
    // @bsiclass                                                    Jeff.Marker     09/2015
    //=======================================================================================
    struct DrawToElementGeometry : IElementGraphicsProcessor
    {
    private:
        TextAnnotationDrawCR m_annotationDraw;
        ElementGeometryBuilderR m_builder;
        DgnCategoryId m_categoryId;
        Transform m_transform;

    public:
        DrawToElementGeometry(TextAnnotationDrawCR annotationDraw, ElementGeometryBuilderR builder, DgnCategoryId categoryId) :
            m_annotationDraw(annotationDraw), m_builder(builder), m_categoryId(categoryId), m_transform(Transform::FromIdentity()) {}

        virtual void _AnnounceTransform(TransformCP transform) override { if (nullptr != transform) { m_transform = *transform; } else { m_transform.InitIdentity(); } }
        virtual void _AnnounceElemDisplayParams(ElemDisplayParamsCR params) override { m_builder.Append(params); }
        virtual BentleyStatus _ProcessTextString(TextStringCR) override;
        virtual BentleyStatus _ProcessCurveVector(CurveVectorCR, bool isFilled) override;
        virtual void _OutputGraphics(ViewContextR) override;
    };

private:
    DEFINE_T_SUPER(RefCountedBase)

    TextAnnotationCP m_annotation;
    Transform m_documentTransform;

    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationDrawCR);

public:
    DGNPLATFORM_EXPORT explicit TextAnnotationDraw(TextAnnotationCR);
    TextAnnotationDraw(TextAnnotationDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextAnnotationDrawR operator=(TextAnnotationDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static TextAnnotationDrawPtr Create(TextAnnotationCR annotation) { return new TextAnnotationDraw(annotation); }
    TextAnnotationDrawPtr Clone() const { return new TextAnnotationDraw(*this); }

    TextAnnotationCR GetAnnotation() const { return *m_annotation; }
    TransformCR GetDocumentTransform() const { return m_documentTransform; }
    void SetDocumentTransform(TransformCR value) { m_documentTransform = value; }

    DGNVIEW_EXPORT BentleyStatus Draw(DgnViewportR, DgnDrawMode, DrawPurpose, DgnCategoryId) const;
    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
    DGNPLATFORM_EXPORT BentleyStatus Draw(ElementGeometryBuilderR, DgnDbR, DgnCategoryId) const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
