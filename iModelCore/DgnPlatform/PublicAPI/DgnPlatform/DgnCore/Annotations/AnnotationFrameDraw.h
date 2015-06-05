//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrameDraw.h $
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

    void CopyFrom(AnnotationFrameDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrameDraw(AnnotationFrameLayoutCR);
    DGNPLATFORM_EXPORT AnnotationFrameDraw(AnnotationFrameDrawCR);
    DGNPLATFORM_EXPORT AnnotationFrameDrawR operator=(AnnotationFrameDrawCR);
    DGNPLATFORM_EXPORT static AnnotationFrameDrawPtr Create(AnnotationFrameLayoutCR);
    DGNPLATFORM_EXPORT AnnotationFrameDrawPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationFrameLayoutCR GetFrameLayout() const;

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
