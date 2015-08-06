//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextBlockDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationTextBlock.h"
#include "AnnotationTextBlockLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationTextBlockDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextBlockDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Used to draw an AnnotationTextBlock.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlockDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationTextBlockLayoutCP m_layout;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationTextBlockDrawCR);
    BentleyStatus DrawTextRun(AnnotationLayoutRunCR, ViewContextR) const;
    BentleyStatus DrawFractionRun(AnnotationLayoutRunCR, ViewContextR) const;
    BentleyStatus DrawLineBreakRun(AnnotationLayoutRunCR, ViewContextR) const;

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlockDraw(AnnotationTextBlockLayoutCR);
    AnnotationTextBlockDraw(AnnotationTextBlockDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationTextBlockDrawR operator=(AnnotationTextBlockDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationTextBlockDrawPtr Create(AnnotationTextBlockLayoutCR layout) { return new AnnotationTextBlockDraw(layout); }
    AnnotationTextBlockDrawPtr Clone() const { return new AnnotationTextBlockDraw(*this); }

    AnnotationTextBlockLayoutCR GetLayout() const { return *m_layout; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
