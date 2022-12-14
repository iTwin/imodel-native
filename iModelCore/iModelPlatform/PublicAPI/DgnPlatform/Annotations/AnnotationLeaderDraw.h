//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once


#include "AnnotationLeaderLayout.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeaderDraw);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeaderDraw);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Used to draw an AnnotationLeader.
//! @ingroup GROUP_Annotation
// @bsiclass
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

    DGNPLATFORM_EXPORT BentleyStatus Draw(Render::GraphicBuilderR, ViewContextR, Render::GeometryParamsR) const;
};

END_BENTLEY_DGN_NAMESPACE
