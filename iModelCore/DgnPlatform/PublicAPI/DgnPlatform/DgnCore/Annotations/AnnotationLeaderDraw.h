//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderDraw.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationLeaderLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeaderDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderDraw);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderDraw : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationLeaderLayoutCP m_leaderLayout;

    void CopyFrom(AnnotationLeaderDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeaderDraw(AnnotationLeaderLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDraw(AnnotationLeaderDrawCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDrawR operator=(AnnotationLeaderDrawCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderDrawPtr Create(AnnotationLeaderLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDrawPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationLeaderLayoutCR GetLeaderLayout() const;

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;

}; // AnnotationLeaderDraw

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
