//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationFrameDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationFrame.h"
#include "AnnotationFrameLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrameDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrameDraw);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Used to draw an AnnotationFrame.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrameDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationFrameLayoutCP m_frameLayout;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFrameDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameDraw(AnnotationFrameLayoutCR);
    AnnotationFrameDraw(AnnotationFrameDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFrameDrawR operator=(AnnotationFrameDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFrameDrawPtr Create(AnnotationFrameLayoutCR frameLayout) { return new AnnotationFrameDraw(frameLayout); }
    AnnotationFrameDrawPtr Clone() const { return new AnnotationFrameDraw(*this); }

    AnnotationFrameLayoutCR GetFrameLayout() const { return *m_frameLayout; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
