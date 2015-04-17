//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextBlockDraw.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationTextBlock.h"
#include "AnnotationTextBlockLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationTextBlockDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextBlockDraw);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Used to draw an AnnotationTextBlock.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlockDraw : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationTextBlockLayoutCP m_layout;

    void CopyFrom(AnnotationTextBlockDrawCR);
    BentleyStatus DrawTextRun(AnnotationLayoutRunCR, ViewContextR) const;
    BentleyStatus DrawFractionRun(AnnotationLayoutRunCR, ViewContextR) const;
    BentleyStatus DrawLineBreakRun(AnnotationLayoutRunCR, ViewContextR) const;

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlockDraw(AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockDraw(AnnotationTextBlockDrawCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockDrawR operator=(AnnotationTextBlockDrawCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextBlockDrawPtr Create(AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockDrawPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationTextBlockLayoutCR GetLayout() const;

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;

}; // AnnotationTextBlockDraw

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
