//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! Used to draw an AnnotationLeader.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeaderDraw : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationLeaderLayoutCP m_leaderLayout;

    void CopyFrom(AnnotationLeaderDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeaderDraw(AnnotationLeaderLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDraw(AnnotationLeaderDrawCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDrawR operator=(AnnotationLeaderDrawCR);
    DGNPLATFORM_EXPORT static AnnotationLeaderDrawPtr Create(AnnotationLeaderLayoutCR);
    DGNPLATFORM_EXPORT AnnotationLeaderDrawPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationLeaderLayoutCR GetLeaderLayout() const;

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
