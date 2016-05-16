/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Annotations/TextAnnotationDraw.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "TextAnnotation.h"

DGNPLATFORM_TYPEDEFS(TextAnnotationDraw);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotationDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Used to draw a TextAnnotation.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct TextAnnotationDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    TextAnnotationCP m_annotation;

    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationDrawCR);

public:
    DGNPLATFORM_EXPORT explicit TextAnnotationDraw(TextAnnotationCR);
    TextAnnotationDraw(TextAnnotationDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextAnnotationDrawR operator=(TextAnnotationDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static TextAnnotationDrawPtr Create(TextAnnotationCR annotation) { return new TextAnnotationDraw(annotation); }
    TextAnnotationDrawPtr Clone() const { return new TextAnnotationDraw(*this); }
    TextAnnotationCR GetAnnotation() const { return *m_annotation; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(Render::GraphicBuilderR graphic, ViewContextR, Render::GeometryParamsR) const;
};

END_BENTLEY_DGN_NAMESPACE
