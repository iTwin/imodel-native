//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotationDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "TextAnnotation.h"

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
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    TextAnnotationCP m_annotation;
    Transform m_documentTransform;

    void CopyFrom(TextAnnotationDrawCR);

public:
    DGNPLATFORM_EXPORT explicit TextAnnotationDraw(TextAnnotationCR);
    DGNPLATFORM_EXPORT TextAnnotationDraw(TextAnnotationDrawCR);
    DGNPLATFORM_EXPORT TextAnnotationDrawR operator=(TextAnnotationDrawCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextAnnotationDrawPtr Create(TextAnnotationCR);
    DGNPLATFORM_EXPORT TextAnnotationDrawPtr Clone() const;

    DGNPLATFORM_EXPORT TextAnnotationCR GetAnnotation() const;
    DGNPLATFORM_EXPORT TransformCR GetDocumentTransform() const;
    DGNPLATFORM_EXPORT void SetDocumentTransform(TransformCR);

    DGNVIEW_EXPORT BentleyStatus Draw(DgnViewportR, DgnDrawMode, DrawPurpose, DgnCategoryId) const;
    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;

}; // TextAnnotationDraw

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
