//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotation.h $ 
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationFrame.h"
#include "AnnotationLeader.h"
#include "AnnotationTextBlock.h"
#include "TextAnnotationSeed.h"

DGNPLATFORM_TYPEDEFS(TextAnnotation);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

typedef bvector<AnnotationLeaderPtr> AnnotationLeaderCollection;
typedef AnnotationLeaderCollection& AnnotationLeaderCollectionR;
typedef AnnotationLeaderCollection const& AnnotationLeaderCollectionCR;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotation : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    DgnProjectP m_project;

    AnnotationTextBlockPtr m_text;
    AnnotationFramePtr m_frame;
    AnnotationLeaderCollection m_leaders;

    void CopyFrom(TextAnnotationCR);
    void Reset();
    void SetTemplateId(DgnStyleId);

public:
    DGNPLATFORM_EXPORT explicit TextAnnotation(DgnProjectR);
    DGNPLATFORM_EXPORT TextAnnotation(TextAnnotationCR);
    DGNPLATFORM_EXPORT TextAnnotationR operator=(TextAnnotationCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT TextAnnotationPtr Clone() const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT void ApplySeed(DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationTextBlockCP GetTextCP() const;
    DGNPLATFORM_EXPORT AnnotationTextBlockP GetTextP();
    DGNPLATFORM_EXPORT void SetText(AnnotationTextBlockCP);
    DGNPLATFORM_EXPORT AnnotationFrameCP GetFrameCP() const;
    DGNPLATFORM_EXPORT AnnotationFrameP GetFrameP();
    DGNPLATFORM_EXPORT void SetFrame(AnnotationFrameCP);
    DGNPLATFORM_EXPORT AnnotationLeaderCollectionCR GetLeaders() const;
    DGNPLATFORM_EXPORT AnnotationLeaderCollectionR GetLeadersR();

}; // TextAnnotation

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
