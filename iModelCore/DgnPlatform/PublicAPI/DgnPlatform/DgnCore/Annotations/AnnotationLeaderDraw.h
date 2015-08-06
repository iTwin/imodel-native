//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeaderDraw.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationLeaderLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeaderDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

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

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLeaderDrawCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeaderDraw(AnnotationLeaderLayoutCR);
    AnnotationLeaderDraw(AnnotationLeaderDrawCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLeaderDrawR operator=(AnnotationLeaderDrawCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLeaderDrawPtr Create(AnnotationLeaderLayoutCR leaderLayout) { return new AnnotationLeaderDraw(leaderLayout); }
    AnnotationLeaderDrawPtr Clone() const { return new AnnotationLeaderDraw(*this); }

    AnnotationLeaderLayoutCR GetLeaderLayout() const { return *m_leaderLayout; }

    DGNPLATFORM_EXPORT BentleyStatus Draw(ViewContextR) const;
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
